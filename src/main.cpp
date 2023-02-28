#include <atomic>
#include <iostream>
#include <unistd.h>
#include <condition_variable>
#include <thread>
#include <time.h>

#include "LDMmap.h"
#include "vehicle-visualizer.h"
#include "QuadKeyTS.h"
#include "AMQPclient.h"
#include "JSONserver.h"
#include "utils.h"
#include "timers.h"

extern "C" {
	#include "options.h"
	#include "CAM.h"
	#include "DENM.h"
}

#include "etsiDecoderFrontend.h"

#define DB_CLEANER_INTERVAL_SECONDS 1
#define DB_DELETE_OLDER_THAN_SECONDS 1 // This value should NEVER be set greater than (5-DB_CLEANER_INTERVAL_SECONDS/60) minutes or (300-DB_CLEANER_INTERVAL_SECONDS) seconds - doing so may break the database age check functionality!

// Global atomic flag to terminate all the threads in case of errors
std::atomic<bool> terminatorFlag;

// Global pointer to a visualizer object (to be accessed by both DBcleaner_callback() and VehVizUpdater_callback())
vehicleVisualizer* globVehVizPtr=nullptr;
// Global mutex (plus condition variable) to synchronize the threads using the object pointer defined above
std::mutex syncmtx;
std::condition_variable synccv;

// Global structure (plus related mutex) to store references to AMQPClient objects to terminate their connections in case of errors
std::unordered_map<int,AMQPClient*> amqpclimap;
std::mutex amqpclimutex;

void AMQPclient_t(ldmmap::LDMMap *db_ptr,options_t *opts_ptr,std::string logfile_name,std::string clientID,unsigned int clientIndex,indicatorTriggerManager *itm_ptr,std::string quadKey_filter,AMQPClient *main_amqp_ptr) {
	if(clientIndex >= MAX_ADDITIONAL_AMQP_CLIENTS-1) {
		fprintf(stderr,"[FATAL ERROR] Error: there is a bug in the code, which attemps to spawn too many AMQP clients.\nPlease report this bug to the developers.\n");
		fprintf(stderr,"Bug details: client id: %s - client index: %u - max supported clients: %u\n",clientID.c_str(),clientIndex,MAX_ADDITIONAL_AMQP_CLIENTS-1);
		terminatorFlag = true;
		return;
	}

	if(opts_ptr->amqp_broker_x[clientIndex].amqp_reconnect_after_local_timeout_expired==true) {
		std::cout << "[AMQPClient "<< clientID << "] This client will be restarted if a local idle timeout error occurs." << std::endl;
	}

	AMQPClient recvClient(std::string(options_string_pop(opts_ptr->amqp_broker_x[clientIndex].broker_url)), std::string(options_string_pop(opts_ptr->amqp_broker_x[clientIndex].broker_topic)), opts_ptr->min_lat,opts_ptr->max_lat, opts_ptr->min_lon, opts_ptr->max_lon, opts_ptr, db_ptr, logfile_name);

	// If this flag is set to true, the client will be restarted after an error, instead of being terminated
	bool cli_restart = false;

	do {
		cli_restart = false;

		try {
			// The indicator trigger manager is disabled by default in AMQPClient, unless it is explicitely enabled with a call to setIndicatorTriggerManager(true)
			if(opts_ptr->indicatorTrgMan_enabled==true) {
				recvClient.setIndicatorTriggerManager(itm_ptr);
			}

			// Set username, if specified
			if(options_string_len(opts_ptr->amqp_broker_x[clientIndex].amqp_username)>0) {
				recvClient.setUsername(std::string(options_string_pop(opts_ptr->amqp_broker_x[clientIndex].amqp_username)));
			}

			// Set password, if specified
			if(options_string_len(opts_ptr->amqp_broker_x[clientIndex].amqp_password)>0) {
				recvClient.setPassword(std::string(options_string_pop(opts_ptr->amqp_broker_x[clientIndex].amqp_password)));
			}

			// Set connection options (they all default to "false" - see also options.c/broker_options_inizialize())
			recvClient.setConnectionOptions(opts_ptr->amqp_broker_x[clientIndex].amqp_allow_sasl,opts_ptr->amqp_broker_x[clientIndex].amqp_allow_insecure,opts_ptr->amqp_broker_x[clientIndex].amqp_reconnect);
			recvClient.setIdleTimeout(opts_ptr->amqp_broker_x[clientIndex].amqp_idle_timeout);

			recvClient.setClientID(clientID);

			// Set the QuadKey filter
			if(opts_ptr->quadkFilter_enabled==true) {
				recvClient.setFilter(quadKey_filter);
			}

			amqpclimutex.lock();
			amqpclimap[clientIndex]=&recvClient;
			amqpclimutex.unlock();

			proton::container(recvClient).run();
		} catch (const std::exception& e) {
			if(opts_ptr->amqp_broker_x[clientIndex].amqp_reconnect_after_local_timeout_expired==true && std::string(e.what()) == "amqp:resource-limit-exceeded: local-idle-timeout expired") {
				std::cerr << "[AMQPClient "<< clientID << "] Exception occurred: " << e.what() << std::endl;
				std::cout << "[AMQPClient "<< clientID << "] Attempting to restart the client after a local idle timeout expired error..." << std::endl;
				recvClient.force_container_stop();
				sleep(1);
				cli_restart = true;
			} else {
				amqpclimutex.lock();
				amqpclimap.erase(clientIndex);
				amqpclimutex.unlock();

				std::cerr << "[AMQPClient "<< clientID << "] Exception occurred: " << e.what() << std::endl;
				terminatorFlag = true;

				main_amqp_ptr->force_container_stop();
			}
		}
	} while(cli_restart==true);

	return;
}

typedef struct vizOptions {
	ldmmap::LDMMap *db_ptr;
	options_t *opts_ptr;
} vizOptions_t;

void clearVisualizerObject(uint64_t id,void *vizObjVoidPtr) {
	vehicleVisualizer *vizObjPtr = static_cast<vehicleVisualizer *>(vizObjVoidPtr);

	vizObjPtr->sendObjectClean(std::to_string(id));
}

void *DBcleaner_callback(void *arg) {
	// Get the pointer to the database
	ldmmap::LDMMap *db_ptr = static_cast<ldmmap::LDMMap *>(arg);

	// Create a new timer
	Timer tmr(DB_CLEANER_INTERVAL_SECONDS*1e3);
	std::cout << "[INFO] Database cleaner started. The DB will be garbage collected every " << DB_CLEANER_INTERVAL_SECONDS << " seconds." << std::endl;

        if(tmr.start()==false) {
                std::cerr << "[ERROR] Fatal error! Cannot create timer for the DB cleaner thread!" << std::endl;
                terminatorFlag = true;
                pthread_exit(nullptr);
        }


	std::unique_lock<std::mutex> synclck(syncmtx);
	synccv.wait(synclck);

	POLL_DEFINE_JUNK_VARIABLE();

	while(terminatorFlag == false && tmr.waitForExpiration()==true) {
			// ---- These operations will be performed periodically ----

			db_ptr->deleteOlderThanAndExecute(DB_DELETE_OLDER_THAN_SECONDS*1e3,clearVisualizerObject,static_cast<void *>(globVehVizPtr));

			// --------
	}

	if(terminatorFlag == true) {
		std::cerr << "[WARN] Database cleaner terminated due to error." << std::endl;
	}

	pthread_exit(nullptr);
}

void updateVisualizer(ldmmap::vehicleData_t vehdata,void *vizObjVoidPtr) {
	vehicleVisualizer *vizObjPtr = static_cast<vehicleVisualizer *>(vizObjVoidPtr);

	vizObjPtr->sendObjectUpdate(std::to_string(vehdata.stationID),vehdata.lat,vehdata.lon,static_cast<int>(vehdata.stationType),vehdata.heading);
}

void *VehVizUpdater_callback(void *arg) {
	// Get the pointer to the visualizer options/parameters
	vizOptions_t *vizopts_ptr = static_cast<vizOptions_t *>(arg);
	// Get a direct pointer to the database
	ldmmap::LDMMap *db_ptr = vizopts_ptr->db_ptr;

	// Get the central lat and lon values stored in the DB
	std::pair<double,double> centralLatLon= db_ptr->getCentralLatLon();

	// Create a new veheicle visualizer object reading the (IPv4) address and port from the options (the default values are set as a macro in options/options.h)
	vehicleVisualizer vehicleVisObj(vizopts_ptr->opts_ptr->vehviz_nodejs_port,std::string(options_string_pop(vizopts_ptr->opts_ptr->vehviz_nodejs_addr)));

	// Start the node.js server and perform an initial connection with it
	vehicleVisObj.setHTTPPort(vizopts_ptr->opts_ptr->vehviz_web_interface_port);
	vehicleVisObj.startServer();
	vehicleVisObj.connectToServer ();
	vehicleVisObj.sendMapDraw(centralLatLon.first, centralLatLon.second,
		vizopts_ptr->opts_ptr->min_lat,vizopts_ptr->opts_ptr->min_lon,
		vizopts_ptr->opts_ptr->max_lat,vizopts_ptr->opts_ptr->max_lon,
		vizopts_ptr->opts_ptr->ext_lat_factor,vizopts_ptr->opts_ptr->ext_lon_factor);

	globVehVizPtr=&vehicleVisObj;

	synccv.notify_all();

	// Create a new timer
	Timer tmr(vizopts_ptr->opts_ptr->vehviz_update_interval_sec*1e3);
	std::cout << "[INFO] Vehicle visualizer updater started. Updated every " << vizopts_ptr->opts_ptr->vehviz_update_interval_sec << " seconds." << std::endl;

        if(tmr.start()==false) {
                std::cerr << "[ERROR] Fatal error! Cannot create timer for the Vehicle Visualizer update thread!" << std::endl;
                terminatorFlag = true;
                pthread_exit(nullptr);
        }

        POLL_DEFINE_JUNK_VARIABLE();

        while(terminatorFlag == false && tmr.waitForExpiration()==true) {

                        // ---- These operations will be performed periodically ----

                        db_ptr->executeOnAllContents(&updateVisualizer, static_cast<void *>(&vehicleVisObj));

			// --------
	}

	if(terminatorFlag == true) {
		std::cerr << "[WARN] Vehicle visualizer updater terminated due to error." << std::endl;
	}

	pthread_exit(nullptr);
}

int main(int argc, char **argv) {
	terminatorFlag = false;

	// DB cleaner thread ID
	pthread_t dbcleaner_tid;
	// Vehicle visualizer update thread ID
	pthread_t vehviz_tid;
	// Thread attributes (unused, for the time being)
	// pthread_attr_t tattr;

	// First of all, parse the options
	options_t sldm_opts;

	// Read options from command line
	options_initialize(&sldm_opts);
	if(parse_options(argc, argv, &sldm_opts)) {
		fprintf(stderr,"Error while parsing the options with the C options module.\n");
		exit(EXIT_FAILURE);
	}

	std::time_t now = std::time(nullptr);
	fprintf(stdout,"[INFO] The S-LDM started at %.24s, corresponding to GNTimestamp = %lu\n",std::ctime(&now),get_timestamp_ms_gn());
	fprintf(stdout,"[INFO] S-LDM version: %s\n",VERSION_STR);

	// Print, as an example, the full (internal + external) area covered by the S-LDM
	std::cout << "This S-LDM instance will cover the full area defined by: [" << 
		sldm_opts.min_lat-sldm_opts.ext_lat_factor << "," << sldm_opts.min_lon-sldm_opts.ext_lon_factor << "],[" <<
		sldm_opts.max_lat+sldm_opts.ext_lat_factor  << "," << sldm_opts.max_lon+sldm_opts.ext_lon_factor << "]" <<
		std::endl;
	if(sldm_opts.cross_border_trigger==true) {
		std::cout << "Cross-border trigger mode enabled." << std::endl;
	}

	/* ----------------- TEST AREA (insert here your test code, which will be removed from the final version of main()) ----------------- */
	/* ------------------------------------------------------------------------------------------------------------------------------------ */
	/* ------------------------------------------------------------------------------------------------------------------------------------ */
	/* ------------------------------------------------------------------------------------------------------------------------------------ */
	/* ------------------------------------------------------------------------------------------------------------------------------------ */
	/* ------------------------------------------------------------------------------------------------------------------------------------ */

	std::cout << "*-*-*-*-*-* [INFO] S-LDM startup auto-tests started... *-*-*-*-*-*" << std::endl;

	// Create a new veheicle visualizer object
	//vehicleVisualizer vehicleVisObj;

	//vehicleVisObj.startServer();
	//vehicleVisObj.connectToServer ();

	// Draw the sample vehicle on the map (simulating 5 updates)
	//vehicleVisObj.sendMapDraw(45.562149, 8.055311);
	//vehicleVisObj.sendObjectUpdate("veh1",45.562149, 8.055311);
	// sleep(1);
	// vehicleVisObj.sendObjectUpdate("veh1",45.562139, 8.055311);
	// sleep(1);
	// vehicleVisObj.sendObjectUpdate("veh1",45.562129, 8.055311);
	// sleep(1);
	// vehicleVisObj.sendObjectUpdate("veh1",45.562119, 8.055311);
	// sleep(1);
	// vehicleVisObj.sendObjectUpdate("veh1",45.562109, 8.055311);

	/* CAM sample (GeoNet,BTP,CAM) (85 bytes) */
	static unsigned char cam[85] = {
		0x11, 0x00, 0x50, 0x01, 0x20, 0x50, 0x02, 0x80, /* ..P. P.. */
		0x00, 0x2d, 0x01, 0x00, 0x14, 0x00, 0x00, 0x00, /* ........ */
		0x00, 0x00, 0x00, 0x03, 0xcf, 0x37, 0x73, 0x6b, /* .....7sk */
		0x1a, 0xda, 0xad, 0xe3, 0x04, 0x90, 0x1e, 0x5e, /* .......^ */
		0x00, 0x01, 0x00, 0xb4, 0x00, 0x00, 0x00, 0x00, /* ........ */
		0x07, 0xd1, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, /* ........ */
		0x00, 0x01, 0xc1, 0xa0, 0x00, 0x5a, 0x0f, 0xef, /* ...C.Z.. */
		0x56, 0x8d, 0xfb, 0x4a, 0x3a, 0x3f, 0xff, 0xff, /* ...>.... */
		0xfc, 0x23, 0xb7, 0x74, 0x3e, 0x20, 0xa8, 0xcf, /* .#.t> p. */
		0xc0, 0x8b, 0x7e, 0x83, 0x18, 0x8a, 0xf3, 0x37, /* .C~....7 */
		0xfe, 0xeb, 0xff, 0xf6, 0x08                   /* .. */
	};

	/* DENM sample (140 bytes) */
	static unsigned char denm[104] = {
		0x11, 0x00, 0xf1, 0x01, 0x20, 0x40, 0x01, 0x00, /* .... @.. */
		0x00, 0x30, 0x01, 0x00, 0x00, 0x02, 0x00, 0x00, /* .0...... */
		0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, /* <....... */
		0x97, 0xe0, 0xf1, 0x5c, 0x1a, 0xda, 0x91, 0x36, /* ...\...6 */
		0x04, 0x90, 0x39, 0x55, 0x00, 0x00, 0x00, 0x00, /* ..9U.... */
		0x1a, 0xda, 0x91, 0x35, 0x04, 0x90, 0x39, 0x55, /* ...5..9U */
		0x00, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* .d...... */
		0x07, 0xd2, 0x00, 0x00, 0x02, 0x01, 0x2e, 0x5d, /* .......] */
		0xa4, 0xe7, 0x20, 0x17, 0x2e, 0xd2, 0x73, 0x80, /* .. ...s. */
		0x00, 0x0f, 0xf2, 0xfc, 0x3f, 0xe8, 0xc0, 0x00, /* ....?... */
		0x00, 0x00, 0x55, 0xd6, 0xb4, 0x9d, 0x20, 0x1d, /* ..U... . */
		0x69, 0x3a, 0x40, 0x10, 0x00, 0x00, 0x00, 0x00, /* i:@..... */
		0xdb, 0xba, 0x1f, 0x0f, 0x08, 0x20, 0x18, 0x00, /* ..... .. */
	};

	unsigned char *ptr = &cam[0];

	// Sample DENM from byte array
	uint8_t denm2_bytes[190];
	std::string denm2_content="11000501204001800086020040EE000014008CFDF01F6D075DE0B1E41B5EA6B506950D1386B801FB1B5EA82D06950FAB01F400000000000007D200000201F01F6D07C7780FB68380020F6BBC1679E3DAEF059E7D103912D71DEE1AB0C80C80001E0788600050141090230D483C7F84826FE283F302C67000C77ECD1F77563380080BF658FBBE31920040DFB297DDF18CE0020AFD96BEEF6C64801037EC89F77663380080BF642FBBB319C00415FB2E7DE318CE00202FD983EF1AC6700102";
	
	for (unsigned int i = 0; i < denm2_content.length(); i += 2) {
		std::string byteString = denm2_content.substr(i, 2);
		uint8_t byte = (uint8_t) strtol(byteString.c_str(), nullptr, 16);
		denm2_bytes[i/2] = byte;
	}

	etsiDecoder::decoderFrontend decodeFrontend;
	etsiDecoder::etsiDecodedData_t decodedData;

	if(decodeFrontend.decodeEtsi((uint8_t *)&denm2_bytes[0], 190, decodedData)!=ETSI_DECODER_OK) {
		std::cerr << "Error! Cannot decode ETSI packet!" << std::endl;
	}

	if(decodedData.type == etsiDecoder::ETSI_DECODED_CAM) {
		CAM_t *decoded_cam;

		decoded_cam = (CAM_t *) decodedData.decoded_msg;

		printf("GNTimestamp: %u\n",decodedData.gnTimestamp);

		printf("Lat: %.7lf, Lon: %.7lf\n",
			(double)decoded_cam->cam.camParameters.basicContainer.referencePosition.latitude/10000000.0,
			(double) decoded_cam->cam.camParameters.basicContainer.referencePosition.longitude/10000000.0);
	} else if(decodedData.type == etsiDecoder::ETSI_DECODED_DENM) {
		DENM_t *decoded_denm;

		decoded_denm = (DENM_t *) decodedData.decoded_msg;

		printf("GNTimestamp: %u\n",decodedData.gnTimestamp);

		printf("GeoArea: \nLat: %.7lf, Lon: %.7lf, DistA %u, DistB %u, Angle %u\n",
				(double)decodedData.posLat/10000000.0,
				(double) decodedData.posLong/10000000.0,
				decodedData.distA,
				decodedData.distB,
				decodedData.angle);
	}

	// Test with a db
	ldmmap::LDMMap dbtest;
	ldmmap::vehicleData_t veh1 = {.stationID=188321312, .lat=45.562149, .lon=8.055311, .elevation=440, .heading=120, .speed_ms=17, .gnTimestamp=34235235235, .timestamp_us=0};
	veh1.timestamp_us = get_timestamp_us(); // now
	dbtest.insert(veh1);
	std::printf("Test vehicle 1 inserted @ %lu\n",veh1.timestamp_us);

	ldmmap::vehicleData_t veh2 = {.stationID=288321312, .lat=45.512149, .lon=8.355311, .elevation=440, .heading=100, .speed_ms=17, .gnTimestamp=34235235235, .timestamp_us=0};
	veh2.timestamp_us = get_timestamp_us()-2*1e6; // 2 seconds ago
	dbtest.insert(veh2);
	std::printf("Test vehicle 2 inserted @ %lu\n",veh2.timestamp_us);

	ldmmap::vehicleData_t veh3 = {.stationID=388321312, .lat=45.592149, .lon=8.855311, .elevation=440, .heading=80, .speed_ms=17, .gnTimestamp=34235235235, .timestamp_us=0};
	veh3.timestamp_us = get_timestamp_us()-5*1e6; // 5 seconds ago
	dbtest.insert(veh3);
	std::printf("Test vehicle 3 inserted @ %lu\n",veh3.timestamp_us);

	ldmmap::vehicleData_t veh4 = {.stationID=488321312, .lat=45.362149, .lon=8.755311, .elevation=440, .heading=10, .speed_ms=17, .gnTimestamp=34235235235, .timestamp_us=0};
	veh4.timestamp_us = get_timestamp_us()-7*1e6; // 7 seconds ago
	dbtest.insert(veh4);
	std::printf("Test vehicle 4 inserted @ %lu\n",veh4.timestamp_us);

	// Print all the contents of the test DB (should be equal to 4)
	dbtest.printAllContents("Before deletion");

	// Print the size of the test DB
	std::cout << "Number of elements stored in the LDMMap DB: " << dbtest.getCardinality() << std::endl;

	// Delete now all the vehicles older than 5.5 seconds
	dbtest.deleteOlderThan(5500); // Only 188321312, 288321312 and 388321312 should remain in the DB

	// Now print all the contents of the DB again
	dbtest.printAllContents("After deletion");

	// Print the size of the test DB again (should be equal to 3)
	std::cout << "Number of elements stored in the LDMMap DB: " << dbtest.getCardinality() << std::endl;

	dbtest.setCentralLatLon(45.562149,8.055311); // Set a central lat lon for testing the visualizer thread

	dbtest.clear();

	std::cout << "*-*-*-*-*-* [INFO] S-LDM startup auto-tests terminated. The S-LDM will start now. *-*-*-*-*-*" << std::endl;

	/* ------------------------------------------------------------------------------------------------------------------------------------ */
	/* ------------------------------------------------------------------------------------------------------------------------------------ */
	/* ------------------------------------------------------------------------------------------------------------------------------------ */
	/* ------------------------------------------------------------------------------------------------------------------------------------ */
	/* ------------------------------------------------------------------------------------------------------------------------------------ */

	// Create a new DB object
	ldmmap::LDMMap *db_ptr = new ldmmap::LDMMap();

	// Set a central latitude and longitude depending on the coverage area of the S-LDM (to be used only for visualization purposes -
	// - it does not affect in any way the performance or the operations of the LDMMap DB module)
	db_ptr->setCentralLatLon((sldm_opts.min_lat+sldm_opts.max_lat)/2.0, (sldm_opts.min_lon+sldm_opts.max_lon)/2.0);

	// Before starting the AMQP client event loop, we should create a parallel thread, reading periodically 
	// (e.g. every 5 s) the database through the pointer "db_ptr" and "cleaning" the entries which are too old
	// pthread_attr_init(&tattr);
	// pthread_attr_setdetachstate(&tattr,PTHREAD_CREATE_DETACHED);
	pthread_create(&dbcleaner_tid,NULL,DBcleaner_callback,(void *) db_ptr);
	// pthread_attr_destroy(&tattr);

	// We should also start here a second parallel thread, reading periodically the database (e.g. every 500 ms) and sending the vehicle data to
	// the vehicleVisualizer
	// pthread_attr_init(&tattr);
	// pthread_attr_setdetachstate(&tattr,PTHREAD_CREATE_DETACHED);
	vizOptions_t vizParams = {db_ptr,&sldm_opts};
	pthread_create(&vehviz_tid,NULL,VehVizUpdater_callback,(void *) &vizParams);
	// pthread_attr_destroy(&tattr);

	// Get the log file name from the options, if available, to enable log mode inside the AMQP client and the S-LDM modules
	std::string logfile_name="";
	if(options_string_len(sldm_opts.logfile_name)>0) {
		logfile_name=std::string(options_string_pop(sldm_opts.logfile_name));
		if(logfile_name!="stdout") {
			time_t rawtime;
			struct tm * timeinfo;
  			char buffer [25] = {NULL};
  			time (&rawtime);	
  			timeinfo = localtime (&rawtime);
  			strftime (buffer,25,"-%Y%m%d-%H:%M:%S",timeinfo);
			logfile_name += buffer;
		}

	}

	// Create an indicatorTriggerManager object (the same object will be then accessed by all the AMQP clients, when using more than one client)
	indicatorTriggerManager itm(db_ptr,&sldm_opts);

	if(sldm_opts.left_indicator_trg_enable==true) {
		itm.setLeftTurnIndicatorEnable(true);
	}

	// Set up the AMQP QuadKey filter for the AMQP client(s) (if more clients are spawned, the filter should be the same for all of them)
	// -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
	QuadKeys::QuadKeyTS tilesys;
	std::string filter_str;
	bool cache_file_found=false;
	tilesys.setLevelOfDetail(16);

	FILE *logfile_file = nullptr;
	uint64_t bf = 0.0,af = 0.0;

	// This is just to log the time needed to compute the full QuadKey filter, if requested by the user
	if(logfile_name!="") {
		if(logfile_name=="stdout") {
			logfile_file=stdout;
		} else {
			// Opening the output file in write + append mode just to be safe in case the user does not change the file name
			// between different executions of the S-LDM
			logfile_file=fopen(logfile_name.c_str(),"wa");
		}

		bf=get_timestamp_ns();
	}

	filter_str=tilesys.getQuadKeyFilter(sldm_opts.min_lat-sldm_opts.ext_lat_factor,sldm_opts.min_lon-sldm_opts.ext_lon_factor,sldm_opts.max_lat+sldm_opts.ext_lat_factor,sldm_opts.max_lon+sldm_opts.ext_lon_factor,&cache_file_found);

	// This is just to log the time needed to compute the full QuadKey filter, if requested by the user
	if(logfile_name!="") {
		af=get_timestamp_ns();

		fprintf(logfile_file,"[LOG - QUADKEY FILTER COMPUTATION] Area (internal)=%.7lf:%.7lf-%.7lf:%7lf Area (full)=%.7lf:%.7lf-%.7lf:%7lf QKCacheFileFound=%d ProcTimeMilliseconds=%.6lf\n",
			sldm_opts.min_lat,sldm_opts.min_lon,sldm_opts.max_lat,sldm_opts.max_lon,
			sldm_opts.min_lat-sldm_opts.ext_lat_factor,sldm_opts.min_lon-sldm_opts.ext_lon_factor,sldm_opts.max_lat+sldm_opts.ext_lat_factor,sldm_opts.max_lon+sldm_opts.ext_lon_factor,
			cache_file_found,(af-bf)/1000000.0);

		if(logfile_name!="stdout" && logfile_file!=nullptr) {
			fclose(logfile_file);
		}
	}
	// -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

	// Create the main AMQP client object
	AMQPClient mainRecvClient(std::string(options_string_pop(sldm_opts.amqp_broker_one.broker_url)), std::string(options_string_pop(sldm_opts.amqp_broker_one.broker_topic)), sldm_opts.min_lat, sldm_opts.max_lat, sldm_opts.min_lon, sldm_opts.max_lon, &sldm_opts, db_ptr, logfile_name);

	// Create the JSONserver object for the on-demand JSON-over-TCP interface
	JSONserver jsonsrv(db_ptr);

	// Start the on-demand JSON-over-TCP interface if enabled through the corresponding option
	if(sldm_opts.od_json_interface_enabled==true) {
		jsonsrv.setServerPort(sldm_opts.od_json_interface_port);
		if(jsonsrv.startServer()!=true) {
			fprintf(stderr,"Critical error: cannot start the JSON server for data retrieval from other services.\n");
			exit(EXIT_FAILURE);
		}
	}

	// Start the AMQP client event loop (additional clients, if requested by the user)
	std::vector<std::thread> amqp_x_threads;

	if(sldm_opts.num_amqp_x_enabled>0) {
		std::cout << "[INFO] Additional AMQP clients will be used by the current instance of the S-LDM. Total number of AMQP clients (including the main one): " << sldm_opts.num_amqp_x_enabled+1 << std::endl;

		for(unsigned int i=0;i<sldm_opts.num_amqp_x_enabled;i++) {
			amqp_x_threads.emplace_back(AMQPclient_t,db_ptr,&sldm_opts,(logfile_name == "stdout" ? "stdout" : logfile_name + std::to_string(i+2)),
				std::to_string(i+2),i,&itm,filter_str,&mainRecvClient);
		}
	}

	if(sldm_opts.amqp_broker_one.amqp_reconnect_after_local_timeout_expired==true) {
		std::cout << "[AMQPClient 1] This client will be restarted if a local idle timeout error occurs." << std::endl;
	}

	// If this flag is set to true, the client will be restarted after an error, instead of being terminated
	bool cli_restart = false;

	do {
		cli_restart = false;

		// Start the AMQP client event loop (main client)
		try {
			// The indicator trigger manager is disabled by default in AMQPClient, unless it is explicitely enabled with a call to setIndicatorTriggerManager(true)
			if(sldm_opts.indicatorTrgMan_enabled==true) {
				mainRecvClient.setIndicatorTriggerManager(&itm);
			}

			// Set username, if specified
			if(options_string_len(sldm_opts.amqp_broker_one.amqp_username)>0) {
				mainRecvClient.setUsername(std::string(options_string_pop(sldm_opts.amqp_broker_one.amqp_username)));
			}

			// Set password, if specified
			if(options_string_len(sldm_opts.amqp_broker_one.amqp_password)>0) {
				mainRecvClient.setPassword(std::string(options_string_pop(sldm_opts.amqp_broker_one.amqp_password)));
			}

			// Set connection options (they all default to "false" - see also options.c/broker_options_inizialize())
			mainRecvClient.setConnectionOptions(sldm_opts.amqp_broker_one.amqp_allow_sasl,sldm_opts.amqp_broker_one.amqp_allow_insecure,sldm_opts.amqp_broker_one.amqp_reconnect);
			mainRecvClient.setIdleTimeout(sldm_opts.amqp_broker_one.amqp_idle_timeout);

			mainRecvClient.setClientID("1");

			// Set the QuadKey filter
			if(sldm_opts.quadkFilter_enabled==true) {
				mainRecvClient.setFilter(filter_str);
			}

			proton::container(mainRecvClient).run();
		} catch (const std::exception& e) {
			if(sldm_opts.amqp_broker_one.amqp_reconnect_after_local_timeout_expired==true && std::string(e.what()) == "amqp:resource-limit-exceeded: local-idle-timeout expired") {
				std::cerr << "[AMQPClient 1] Exception occurred: " << e.what() << std::endl;
				std::cout << "[AMQPClient 1] Attempting to restart the client after a local idle timeout expired error..." << std::endl;
				mainRecvClient.force_container_stop();
				sleep(1);
				cli_restart = true;
			} else {
				std::cerr << e.what() << std::endl;
				terminatorFlag = true;
			}
		}
	} while(cli_restart==true);

	pthread_join(dbcleaner_tid,nullptr);
	pthread_join(vehviz_tid,nullptr);

	if(sldm_opts.num_amqp_x_enabled>0) {
		fprintf(stdout,"[INFO] Terminating the other AMQP clients...\n");
		// Close the connection on all the other brokers (if multiple clients are used)
		amqpclimutex.lock();
		for (auto const& [key, val] : amqpclimap) {
			fprintf(stdout,"[INFO] Terminating client %d...\n",key+2);
			val->force_container_stop();
		}
		amqpclimutex.unlock();
	}

	// Joining threads from additional AMQP clients
	for(std::vector<std::thread>::size_type i=0;i<amqp_x_threads.size();i++) {
		amqp_x_threads[i].join();
	}

	db_ptr->clear();

	// Freeing the options
	options_free(&sldm_opts);

	return 0;
}
