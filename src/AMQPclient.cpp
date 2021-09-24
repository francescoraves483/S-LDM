#include "AMQPclient.h"
#include "QuadKeyTS.h"
#include "utils.h"
#include <fstream>
#include <iomanip>
#include <proton/reconnect_options.hpp>

extern "C" {
	#include "CAM.h"
}

namespace {
	// Example custom function to configure an AMQP filter,
	// specifically an APACHE.ORG:SELECTOR
	// (http://www.amqp.org/specification/1.0/filters)

	void set_filter(proton::source_options &opts, const std::string& selector_str) {

		proton::source::filter_map map;
		proton::symbol filter_key("selector");
		proton::value filter_value;


		// The value is a specific AMQP "described type": binary string with symbolic descriptor
		proton::codec::encoder enc(filter_value);
		enc << proton::codec::start::described()
			<< proton::symbol("apache.org:selector-filter:string")
			<< selector_str
			<< proton::codec::finish();

		// In our case, the map has this one element
		map.put(filter_key, filter_value);
		opts.filters(map);
	}
}

void 
AMQPClient::on_connection_open(proton::connection &conn) {
	if(m_logfile_name!="" && m_logfile_file!=nullptr) {
		fprintf(m_logfile_file,"[LOG - AMQPClient %s] Connection successfully established.\n",m_client_id.c_str());
		fflush(m_logfile_file);
	}
}

void 
AMQPClient::on_connection_close(proton::connection &conn) {
	if(m_logfile_name!="" && m_logfile_file!=nullptr) {
		fprintf(m_logfile_file,"[LOG - AMQPClient %s] Connection closed.\n",m_client_id.c_str());
		fflush(m_logfile_file);
	}
}

void 
AMQPClient::on_container_start(proton::container &c) {
	m_cont.store(&c);

	proton::source_options opts;
	proton::connection_options co;
	bool co_set = false;

	// std::vector<std::string> quadKeys;
	// std::vector<std::string> fromfile;
	// QuadKeys::QuadKeyTS tilesys;

	// Set the connection options
	if(!m_username.empty()) {
		co.user(m_username);
		co_set = true;

		std::cout<<"[AMQPClient " << m_client_id.c_str() << "] AMQP username successfully set: "<<m_username<<std::endl;
	}

	if(!m_password.empty()) {
		co.password(m_password);
		co_set = true;

		std::cout<<"[AMQPClient " << m_client_id.c_str() << "] AMQP password successfully set."<<std::endl;
	}

	if(m_reconnect == true) {
		co.reconnect(proton::reconnect_options());
		co_set = true;

		std::cout<<"[AMQPClient " << m_client_id.c_str() << "] AMQP automatic reconnection enabled."<<std::endl;
	}

	if(m_allow_sasl == true) {
		co.sasl_enabled(true);
		co_set = true;

		std::cout<<"[AMQPClient " << m_client_id.c_str() << "] AMQP SASL enabled."<<std::endl;
	}

	if(m_allow_insecure == true) {
		co.sasl_allow_insecure_mechs(true);
		co_set = true;

		std::cout<<"[AMQPClient " << m_client_id.c_str() << "] Warning: clear-text passwords are enabled."<<std::endl;
	}

	if(m_idle_timeout_ms>=0) {
		if(m_idle_timeout_ms==0) {
			co.idle_timeout(proton::duration::FOREVER);

			std::cout<<"[AMQPClient " << m_client_id.c_str() << "] Idle timeout set to FOREVER."<<std::endl;
		} else {
			co.idle_timeout(proton::duration(m_idle_timeout_ms));

			std::cout<<"[AMQPClient " << m_client_id.c_str() << "] Idle timeout set to "<<m_idle_timeout_ms<<"."<<std::endl;
		}

		co_set = true;
	} else {
		std::cout<<"[AMQPClient " << m_client_id.c_str() << "] No idle timeout has been explicitely set."<<std::endl;
	}

	if(m_logfile_name!="") {
		if(m_logfile_name=="stdout") {
			m_logfile_file=stdout;
		} else {
			// Opening the output file in write + append mode just to be safe in case the user does not change the file name
			// between different executions of the S-LDM
			m_logfile_file=fopen(m_logfile_name.c_str(),"wa");
		}
	}

	/* First version of the code without the caching mechanism. Kept here for reference. */
	/*
		//LevelOfDetail set into the tilesys class as private variables
		tilesys.setLevelOfDetail(levelOfdetail);

		std::cout << "[AMQP Client] Quadkey algoritm started." << std::endl;

		//here we get the vector containing all the quadkeys in the range at a given level of detail
		quadKeys = tilesys.LatLonToQuadKeyRange(min_latitude, max_latitude, min_longitude, max_longitude);
		//Quadkeys unifier algorithm
		tilesys.unifyQuadkeys(quadKeys);

		//Here we create a string to pass to the filter (SQL like)
		std::string s;

		for(size_t i = 0; i < quadKeys.size(); i++) {
			s.insert(s.length(), "quadkeys LIKE ''");
			//l = s.length() - 1;
			s.insert(s.length() - 1, quadKeys.at(i));
			s.insert(s.length() - 1, "%");
			if(i < quadKeys.size()-1){
				s.insert(s.length(), " OR ");
			}
		}

		std::cout << "[AMQP Client] Quadkey algoritm correctly terminated." << std::endl;

		// Set the AMQP filter
		set_filter(opts, s);
	*/

	/* Second version of the code without the caching mechanism. Kept here for reference. */
	// This code has been moved into a dedicated function inside QuadKeyTS.h/.cpp (getQuadKeyFilter())
	//
	// std::string line;
	// bool cache_file_found = false;
	// std::ifstream ifile("cachefile.sldmc");
	// uint64_t bf = 0.0,af = 0.0;

	// if(m_logfile_name!="") {
	// 	bf=get_timestamp_ns();
	// }

	// if(ifile.is_open()) {
	// 	std::cout<<"[AMQPClient " << m_client_id.c_str() << "] Cache file available: reading the parameters..."<< std::endl;

	// 	while(getline(ifile, line)) {
	// 		fromfile.push_back(line);
	// 	}

	// 	double minlatff = stod(fromfile.at(0));
	// 	double maxlatff = stod(fromfile.at(1));
	// 	double minlonff = stod(fromfile.at(2));
	// 	double maxlonff = stod(fromfile.at(3));

	// 	// fprintf(stdout,"From File we get max_latitude: %.40lf\n",maxlatff);
	// 	// fprintf(stdout,"Actual max_latitude parameter%.40lf\n",max_latitude);
	// 	// fprintf(stdout,"From File we get min_latitude: %.40lf\n",minlatff);
	// 	// fprintf(stdout,"Actual min_latitude parameter%.40lf\n",min_latitude);

	// 	if(doublecomp(minlatff, min_latitude) && doublecomp(maxlatff, max_latitude) && doublecomp(minlonff, min_longitude) && doublecomp(maxlonff, max_longitude) && fromfile.size() > 4){
	// 		cache_file_found = true;
	// 	}
	// } else {
	// 	std::cout<<"[AMQPClient " << m_client_id.c_str() << "] No cache file found!"<<std::endl;
	// }

	// ifile.close();

	// if(cache_file_found == false) {
	// 	std::ofstream ofile("cachefile.sldmc");

	// 	std::cout<<"[AMQPClient " << m_client_id.c_str() << "] New coordinates: recomputing quadkeys..."<<std::endl;
	// 	//LevelOfDetail set into the tilesys class as private variables
	// 	tilesys.setLevelOfDetail(levelOfdetail);
	// 	// Here we get the vector containing all the quadkeys in the range at a given level of detail
	// 	quadKeys = tilesys.LatLonToQuadKeyRange(min_latitude, max_latitude, min_longitude, max_longitude);

	// 	// Add the range information to the cache file
	// 	if(ofile.is_open()) {
	// 		ofile << std::fixed << std::setprecision(6) << min_latitude << "\n" << max_latitude << "\n" << min_longitude << "\n" << max_longitude << "\n";
	// 	}

	// 	// Quadkeys unifier algorithm
	// 	// tilesys.unifyQuadkeys(quadKeys);
	// 	quadKeys=tilesys.unifyQuadkeys2(quadKeys);
	// 	tilesys.checkdim(quadKeys);

	// 	// Write the computed Quadkeys to the cache file
	// 	std::ofstream file;
	// 	if(ofile.is_open()) {
	// 		for(size_t i = 0; i < quadKeys.size(); i++){
	// 			ofile << quadKeys.at(i) << "\n";
	// 		}
	// 	}

	// 	ofile.close();

	// 	std::cout<<"[AMQP Client " << m_client_id.c_str() << "] Finished: Quadkey cache file created."<<std::endl;

	// 	// Here we create a string to pass to the filter (SQL like)
	// 	std::string s;

	// 	for(size_t i = 0; i < quadKeys.size(); i++) {
	// 		s.insert(s.length(), "quadkeys LIKE ''");
	// 		//l = s.length() - 1;
	// 		s.insert(s.length() - 1, quadKeys.at(i));
	// 		s.insert(s.length() - 1, "%");
	// 		if(i < quadKeys.size()-1){
	// 			s.insert(s.length(), " OR ");
	// 		}
	// 	}

	// 	// Set the AMQP filter
	// 	set_filter(opts, s);
	// } else {
	// 	std::cout<<"[AMQPClient " << m_client_id.c_str() << "] Filter setup from a cache file... "<<std::endl;

	// 	std::string s;

	// 	for(size_t i = 4; i < fromfile.size(); i++) {
	// 		s.insert(s.length(), "quadkeys LIKE ''");
	// 		//l = s.length() - 1;
	// 		s.insert(s.length() - 1, fromfile.at(i));
	// 		s.insert(s.length() - 1, "%");
	// 		if(i < fromfile.size()-1){
	// 			s.insert(s.length(), " OR ");
	// 		}
	// 	}

	// 	// Set the AMQP filter
	// 	set_filter(opts, s);
	// }

	// if(m_logfile_name!="") {
	// 	af=get_timestamp_ns();

	// 	fprintf(m_logfile_file,"[LOG - AMQP STARTUP (Client %s)] Area=%.7lf:%.7lf-%.7lf:%7lf QKCacheFileFound=%d ProcTimeMilliseconds=%.6lf\n",
	// 		m_client_id.c_str(),
	// 		min_latitude,min_longitude,max_latitude,max_longitude,
	// 		cache_file_found,(af-bf)/1000000.0);
	// }

	if(m_quadKey_filter!="") {
		set_filter(opts,m_quadKey_filter);

		std::cout << "[AMQPClient " << m_client_id.c_str() << "] QuadKey filter successfully set." << std::endl;
	} else {
		std::cout << "[AMQPClient " << m_client_id.c_str() << "] QuadKey filter not set. Any message will be received." << std::endl;
	}

	std::cout << "[AMQPClient " << m_client_id.c_str() << "] Connecting to AMQP broker at: " << conn_url_ << std::endl;

	proton::connection conn;
	if(co_set == true) {
		std::cout << "[AMQPClient " << m_client_id.c_str() << "] Connecting with user-defined connection options." << std::endl;
		conn = c.connect(conn_url_,co);
	} else {
		std::cout << "[AMQPClient " << m_client_id.c_str() << "] Connecting with default connection options." << std::endl;
		conn = c.connect(conn_url_);
	}

	if(m_quadKey_filter!="") {
		conn.open_receiver(addr_, proton::receiver_options().source(opts));
	} else{
		conn.open_receiver(addr_);
	}
}

void 
AMQPClient::on_message(proton::delivery &d, proton::message &msg) {
	etsiDecoder::etsiDecodedData_t decodedData;

	uint64_t bf = 0.0,af = 0.0;
	uint64_t main_bf = 0.0,main_af = 0.0;

	if(m_logfile_name!="") {
		main_bf=get_timestamp_ns();

		// This additional log line has been commented out to avoid being too verbose
		// fprintf(m_logfile_file,"[NEW MESSAGE RX]\n");
	}

	if(m_printMsg == true) {
		std::cout << msg.body() << std::endl;
	}

	proton::codec::decoder qpid_decoder(msg.body());
	proton::binary message_bin;
	uint8_t *message_bin_buf;

	// Check if a binary message has been received
	// If no binary data has been received, just ignore the current AMQP message
	if(qpid_decoder.next_type () == proton::BINARY) {
		qpid_decoder >> message_bin;

		message_bin_buf=message_bin.data ();
	} else {
		// This message should be ignored
		if(m_printMsg == true) {
			std::cerr << "Error: received a message in a non-binary AMQP type." << std::endl;
		}
		return;
	}

	if(m_logfile_name!="") {
		bf=get_timestamp_ns();
	}

	// Decode the content of the message, using the decoder-module frontend class
	if(m_decodeFrontend.decodeEtsi(message_bin_buf, message_bin.size (), decodedData)!=ETSI_DECODER_OK) {
		std::cerr << "Error! Cannot decode ETSI packet!" << std::endl;
		return;
	}

	if(m_logfile_name!="") {
		af=get_timestamp_ns();

		fprintf(m_logfile_file,"[LOG - MESSAGE DECODER (Client %s)] ProcTimeMilliseconds=%.6lf\n",m_client_id.c_str(),(af-bf)/1000000.0);
	}

	// If a CAM has been received, it should be used to update the internal in-memory database
	if(decodedData.type == etsiDecoder::ETSI_DECODED_CAM) {
		CAM_t *decoded_cam = (CAM_t *) decodedData.decoded_msg;
		double lat = decoded_cam->cam.camParameters.basicContainer.referencePosition.latitude/10000000.0;
		double lon = decoded_cam->cam.camParameters.basicContainer.referencePosition.longitude/10000000.0;
		uint32_t stationID = decoded_cam->header.stationID;
		double l_inst_period=0.0;

		// After getting the lat and lon values from the CAM, check if it is inside the S-LDM full coverage area,
		// using the areaFilter module (which can access the command line options, thus also the coverage area
		// specified by the user)
		if(m_logfile_name!="") {
			bf=get_timestamp_ns();
		}

		if(m_areaFilter.isInside(lat,lon)==false) {
			return;
		}

		if(m_logfile_name!="") {
			af=get_timestamp_ns();

			fprintf(m_logfile_file,"[LOG - AREA FILTER (Client %s)] ProcTimeMilliseconds=%.6lf\n",m_client_id.c_str(),(af-bf)/1000000.0);
		}

		if(m_logfile_name!="") {
			bf=get_timestamp_ns();
		}

		// Update the database
		ldmmap::vehicleData_t vehdata;
		ldmmap::LDMMap::LDMMap_error_t db_retval;

		// Check the age of the data store inside the database (if the age check is enabled / -g option not specified)
		// before updating it with the new receive data
		if(m_opts_ptr->ageCheck_enabled == true) {
			ldmmap::LDMMap::returnedVehicleData_t retveh;

			if(m_db_ptr->lookup(stationID,retveh)==ldmmap::LDMMap::LDMMAP_OK) {
				// According to the standard: GNTimestamp = (TAI timestamp since 2004-01-01 00:00:00) % 4294967296
				// Due to the modulo operation, it is not enough to consider the difference between the received GNTimestamp and the one
				// stored in the database, as this may cause issues when receiving data and the GNTimestamp values are cyclically reset
				// at the same time
				// We thus check the "gap" between the received numbers. Let's consider for instance: stored=4294967291, rx=3
				// In this case the "rx" data is the most up-to-date, but a cyclical reset occurred
				// We can then compute gap = 3 - 4294967291 = -429467288 < -300000 (-5 minutes) - ok! We keep this data even if 3 < 4294967291
				// Let's consider instead:
				// stored=3, rx=4294967291
				// In this case 'rx' is not the most up to date data (it is impossible to have '3' stored in the database and then receive
				// '4294967291', unless clock jumps occur in the car, bacause after all that time the data corresponding to '3' would have already
				// been garbage cleaned from the database)
				// We can then compute gap = 4294967291 - 3 = 429467288 > 300000 (5 minutes) - no! We should dicard the data we just received
				// Let's consider now a "normal" scenario:
				// stored=3, rx=114
				// gap = 114 - 3 = 111 < 300000 - ok! The data is kept (it would be discarded only if gap > 300000)
				// Finally, let's briefly analyze a final scenario:
				// stored=4294967292, rx=4294967291
				// It is evident how the rx data should be discarded because older than the stored one
				// gap = rx - stored = 4294967291 - 4294967292 = -1 > -300000 (-5 minutes) - The data is correctly discarded due to the second
				// condition in the if() clause
				long long int gap = static_cast<long long int>(decodedData.gnTimestamp)-static_cast<long long int>(retveh.vehData.gnTimestamp);

				if((decodedData.gnTimestamp>retveh.vehData.gnTimestamp && gap>300000) ||
					(decodedData.gnTimestamp<retveh.vehData.gnTimestamp && gap>-300000)) {
					if(m_logfile_name!="") {
						fprintf(m_logfile_file,"[LOG - DATABASE UPDATE (Client %s)] Message discarded (data is too old). Rx = %u, Stored = %lu, Gap = %lld\n",
							m_client_id.c_str(),
							decodedData.gnTimestamp,retveh.vehData.gnTimestamp,gap);
						return;
					}
				}
			}
		}

		vehdata.lon = lon;
		vehdata.lat = lat;
		vehdata.timestamp_us = get_timestamp_us();
		vehdata.elevation = decoded_cam->cam.camParameters.basicContainer.referencePosition.altitude.altitudeValue/100.0;
		vehdata.heading = decoded_cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.heading.headingValue/10.0;
		vehdata.speed_ms = decoded_cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.speed.speedValue/100.0;
		vehdata.gnTimestamp = decodedData.gnTimestamp;
		vehdata.stationID = stationID; // It is very important to save also the stationID
		vehdata.camTimestamp = static_cast<long>(decoded_cam->cam.generationDeltaTime);
		vehdata.stationType = static_cast<ldmmap::e_StationTypeLDM>(decoded_cam->cam.camParameters.basicContainer.stationType);

		// Save also the source vehicle quadkey
		if(msg.properties().size()>0) {
			proton::scalar quadkey_prop = msg.properties().get("quadkeys");

			if(quadkey_prop.type() == proton::STRING) {
				vehdata.sourceQuadkey = proton::get<std::string>(quadkey_prop);
			} else {
				vehdata.sourceQuadkey="";
			}
		} else {
			vehdata.sourceQuadkey="";
		}

		if(decoded_cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.vehicleWidth != VehicleWidth_unavailable) {
			vehdata.vehicleWidth = ldmmap::OptionalDataItem<long>(decoded_cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.vehicleWidth*100);
		} else {
			vehdata.vehicleWidth = ldmmap::OptionalDataItem<long>(false);
		}

		if(decoded_cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.vehicleLength.vehicleLengthValue != VehicleLengthValue_unavailable) {
			vehdata.vehicleLength = ldmmap::OptionalDataItem<long>(decoded_cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.vehicleLength.vehicleLengthValue*100);
		} else {
			vehdata.vehicleLength = ldmmap::OptionalDataItem<long>(false);
		}

		// Manage the low frequency container data
		// Check if this CAM contains the low frequency container
		// If yes, store the exterior lights status
		// If not, check if an older information about the exterior lights of the current vehicle already exist in the database (using m_db_ptr->lookup()),
		// if this data exists, use this data, if not, just set the exterior lights information as unavailable
		if(decoded_cam->cam.camParameters.lowFrequencyContainer!=NULL) {
			// In any normal, uncorrupted CAM, buf should never be NULL and it should contain at least one element (i.e. buf[0] always exists)
			if(decoded_cam->cam.camParameters.lowFrequencyContainer->choice.basicVehicleContainerLowFrequency.exteriorLights.buf!=NULL) {
				vehdata.exteriorLights = ldmmap::OptionalDataItem<uint8_t>(decoded_cam->cam.camParameters.lowFrequencyContainer->choice.basicVehicleContainerLowFrequency.exteriorLights.buf[0]);
			} else {
				// Data from a corrupted decoded CAM is considered as unavailable, for the time being
				vehdata.exteriorLights = ldmmap::OptionalDataItem<uint8_t>(false);
			}
		} else {
			ldmmap::LDMMap::returnedVehicleData_t retveh;

			if(m_db_ptr->lookup(stationID,retveh)==ldmmap::LDMMap::LDMMAP_OK) {
				vehdata.exteriorLights = retveh.vehData.exteriorLights;
			} else {
				vehdata.exteriorLights = ldmmap::OptionalDataItem<uint8_t>(false);
			}
		}

		// If logging is enabled, compute also an "instantaneous update period" metric (i.e., how much time has passed between two consecutive vehicle updates)
		if(m_logfile_name!="") {
			ldmmap::LDMMap::returnedVehicleData_t retveh;

			if(m_db_ptr->lookup(stationID,retveh)==ldmmap::LDMMap::LDMMAP_OK) {
				l_inst_period=(get_timestamp_us()-retveh.vehData.timestamp_us)/1000.0;
			} else {
				l_inst_period=-1.0;
			}
			
		}

		// std::cout << "[DEBUG] Updating vehicle with stationID: " << vehdata.stationID << std::endl;

		db_retval=m_db_ptr->insert(vehdata);

		if(db_retval!=ldmmap::LDMMap::LDMMAP_OK && db_retval!=ldmmap::LDMMap::LDMMAP_UPDATED) {
			std::cerr << "Warning! Insert on the database for vehicle " << (int) stationID << "failed!" << std::endl;
		}

		if(m_logfile_name!="") {
			af=get_timestamp_ns();

			fprintf(m_logfile_file,"[LOG - DATABASE UPDATE (Client %s)] LowFrequencyContainerAvail=%d InsertReturnValue=%d ProcTimeMilliseconds=%.6lf\n",
				m_client_id.c_str(),
				decoded_cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.vehicleWidth != VehicleWidth_unavailable,
				db_retval,
				(af-bf)/1000000.0);
		}

		if(m_logfile_name!="") {
			bf=get_timestamp_ns();
		}

		// If a trigger manager has been enabled, check if any triggering condition has occurred (for the time being, only a simple trigger manager based on turn indicators has been developed)
		if(m_indicatorTrgMan_enabled == true && vehdata.exteriorLights.isAvailable()) {
			// Trigger either if the cross-border trigger mode is enabled or if the triggering vehicle is located inside the internal area of this S-LDM instance
			if(m_opts_ptr->cross_border_trigger==true || m_areaFilter.isInsideInternal(lat,lon)==true) {
				if(m_indicatorTrgMan_ptr->checkAndTrigger(lat,lon,stationID,vehdata.exteriorLights.getData()) == true) {
					std::cout << "[TRIGGER] Triggering condition detected!" << std::endl;
				}
			}
		}

		if(m_logfile_name!="") {
			af=get_timestamp_ns();

			fprintf(m_logfile_file,"[LOG - TRIGGER CHECK (Client %s)] TriggerEnabled=%d ExteriorLightsAvail=%d CrossBrdTriggerMode=%d IsInsideInternalArea=%d ProcTimeMilliseconds=%.6lf\n",
				m_client_id.c_str(),
				m_indicatorTrgMan_enabled,
				vehdata.exteriorLights.isAvailable(),
				m_opts_ptr->cross_border_trigger,
				m_areaFilter.isInsideInternal(lat,lon),
				(af-bf)/1000000.0);
		}

		ASN_STRUCT_FREE(asn_DEF_CAM,decoded_cam);

		if(m_logfile_name!="") {
			main_af=get_timestamp_ns();

			logfprintf(m_logfile_file,std::string("FULL CAM PROCESSING (Client") + m_client_id + std::string(")"),"StationID=%u Coordinates=%.7lf:%.7lf Heading=%.1lf InstUpdatePeriod=%.3lf"
				" CAMTimestamp=%ld GNTimestamp=%lu CAMTimestampDiff=%ld GNTimestampDiff=%ld"
				" ProcTimeMilliseconds=%.6lf\n",
				stationID,lat,lon,
				vehdata.heading,
				l_inst_period,
				vehdata.camTimestamp,vehdata.gnTimestamp,get_timestamp_ms_cam()-vehdata.camTimestamp,get_timestamp_ms_gn()-vehdata.gnTimestamp,
				(main_af-main_bf)/1000000.0);
			
			// fprintf(m_logfile_file,"[LOG - FULL CAM PROCESSING] StationID=%u Coordinates=%.7lf:%.7lf InstUpdatePeriod=%.3lf"
			// 	" CAMTimestamp=%ld GNTimestamp=%lu CAMTimestampDiff=%ld GNTimestampDiff=%ld"
			// 	" ProcTimeMilliseconds=%.6lf\n",
			// 	stationID,lat,lon,
			// 	l_inst_period,
			// 	vehdata.camTimestamp,vehdata.gnTimestamp,get_timestamp_ms_cam()-vehdata.camTimestamp,get_timestamp_ms_gn()-vehdata.gnTimestamp,
			// 	(main_af-main_bf)/1000000.0);	
		}

	} else {
		std::cerr << "Warning! Only CAM messages are supported for the time being!" << std::endl;
		return;
	}
}

void 
AMQPClient::on_container_stop(proton::container &c) {
	if(m_logfile_name!="" && m_logfile_name!="stdout") {
		fclose(m_logfile_file);
	}
}