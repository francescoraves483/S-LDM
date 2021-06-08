#include <atomic>
#include <iostream>
#include <unistd.h>

#include "LDMmap.h"
#include "vehicle-visualizer.h"
#include "QuadKeyTS.h"
#include "AMQPclient.h"
#include "utils.h"

extern "C" {
	#include "options.h"
	#include "CAM.h"
	#include "DENM.h"
}

#include "etsiDecoderFrontend.h"

#define DB_CLEANER_INTERVAL_SECONDS 5
#define DB_DELETE_OLDER_THAN_SECONDS 7
#define VEHVIZ_UPDATE_INTERVAL_SECONDS 0.5

// Global atomic flag to terminate all the threads in case of errors
std::atomic<bool> terminatorFlag;

void *DBcleaner_callback(void *arg) {
	// Get the pointer to the database
	ldmmap::LDMMap *db_ptr = static_cast<ldmmap::LDMMap *>(arg);

	// Create a new timer
	struct pollfd pollfddata;
	int clockFd;

	std::cout << "[INFO] Database cleaner started. The DB will be garbage collected every " << DB_CLEANER_INTERVAL_SECONDS << " seconds." << std::endl;

	if(timer_fd_create(pollfddata, clockFd, DB_CLEANER_INTERVAL_SECONDS*1e6)<0) {
		std::cerr << "[ERROR] Fatal error! Cannot create timer for the DB cleaner thread!" << std::endl;
		terminatorFlag = true;
		pthread_exit(nullptr);
	}

	POLL_DEFINE_JUNK_VARIABLE();

	while(terminatorFlag == false) {
		if(poll(&pollfddata,1,0)>0) {
			POLL_CLEAR_EVENT(clockFd);

			// ---- These operations will be performed periodically ----

			db_ptr->deleteOlderThan(DB_DELETE_OLDER_THAN_SECONDS*1e3);

			// --------

		}
	}

	if(terminatorFlag == true) {
		std::cerr << "[WARN] Database cleaner terminated due to error." << std::endl;
	}

	close(clockFd);

	pthread_exit(nullptr);
}

void updateVisualizer(ldmmap::vehicleData_t vehdata,void *vizObjVoidPtr) {
	vehicleVisualizer *vizObjPtr = static_cast<vehicleVisualizer *>(vizObjVoidPtr);

	vizObjPtr->sendObjectUpdate(std::to_string(vehdata.stationID),vehdata.lat,vehdata.lon,vehdata.heading);
}

void *VehVizUpdater_callback(void *arg) {
	// Get the pointer to the database
	ldmmap::LDMMap *db_ptr = static_cast<ldmmap::LDMMap *>(arg);

	// Get the central lat and lon values stored in the DB
	std::pair<double,double> centralLatLon= db_ptr->getCentralLatLon();

	// Create a new veheicle visualizer object
	vehicleVisualizer vehicleVisObj;

	// Start the node.js server and perform an initial connection with it
	vehicleVisObj.startServer();
	vehicleVisObj.connectToServer ();
	vehicleVisObj.sendMapDraw(centralLatLon.first, centralLatLon.second);

	// Create a new timer
	struct pollfd pollfddata;
	int clockFd;

	std::cout << "[INFO] Vehicle visualizer updater started. Updated every " << VEHVIZ_UPDATE_INTERVAL_SECONDS << " seconds." << std::endl;

	if(timer_fd_create(pollfddata, clockFd, VEHVIZ_UPDATE_INTERVAL_SECONDS*1e6)<0) {
		std::cerr << "[ERROR] Fatal error! Cannot create timer for the Vehicle Visualizer update thread!" << std::endl;
		terminatorFlag = true;
		pthread_exit(nullptr);
	}

	POLL_DEFINE_JUNK_VARIABLE();

	while(terminatorFlag == false) {
		if(poll(&pollfddata,1,0)>0) {
			POLL_CLEAR_EVENT(clockFd);

			// ---- These operations will be performed periodically ----

			db_ptr->executeOnAllContents(&updateVisualizer, static_cast<void *>(&vehicleVisObj));

			// --------

		}
	}

	if(terminatorFlag == true) {
		std::cerr << "[WARN] Vehicle visualizer updater terminated due to error." << std::endl;
	}

	close(clockFd);

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

	// Print, as an example, the full (internal + external) area covered by the S-LDM
	std::cout << "This S-LDM instance will cover the full area defined by: [" << 
		sldm_opts.min_lat-sldm_opts.ext_lat_factor << "," << sldm_opts.min_lon-sldm_opts.ext_lon_factor << "],[" <<
		sldm_opts.max_lat+sldm_opts.ext_lat_factor  << "," << sldm_opts.max_lon+sldm_opts.ext_lon_factor << "]" <<
		std::endl;
	if(sldm_opts.cross_border_trigger==true) {
		std::cout << "Cross-border trigger mode enabled." << std::endl;
	}

	/* ----------------- TEST AREA (insert here your test code, which will be removed from the final version of main() ----------------- */
	/* ------------------------------------------------------------------------------------------------------------------------------------ */
	/* ------------------------------------------------------------------------------------------------------------------------------------ */
	/* ------------------------------------------------------------------------------------------------------------------------------------ */
	/* ------------------------------------------------------------------------------------------------------------------------------------ */
	/* ------------------------------------------------------------------------------------------------------------------------------------ */

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
	pthread_create(&vehviz_tid,NULL,VehVizUpdater_callback,(void *) db_ptr);
	// pthread_attr_destroy(&tattr);

	// Start the AMQP client event loop (for the time being, on loopback, but some options will be added in the future)
	try {
		std::string conn_url = argc > 1 ? argv[1] : "127.0.0.1:5672";
		std::string addr = argc > 2 ? argv[2] : "examples";


		AMQPClient recvClient("127.0.0.1:5672", "examples", sldm_opts.min_lat, sldm_opts.max_lat, sldm_opts.min_lon, sldm_opts.max_lon, 16, &sldm_opts, db_ptr);
		proton::container(recvClient).run();

		return 0;
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		terminatorFlag = true;
	}

	pthread_join(dbcleaner_tid,nullptr);
	pthread_join(vehviz_tid,nullptr);

	db_ptr->clear();

	return 0;
}