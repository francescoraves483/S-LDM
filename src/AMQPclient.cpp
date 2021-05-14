#include "AMQPclient.h"
#include "QuadKeyTS.h"

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
AMQPClient::on_container_start(proton::container &c) {
	proton::source_options opts;
	QuadKeys::QuadKeyTS tilesys;
	std::vector<std::string> quadKeys;

	// std::ofstream fout("finalvec.txt");

	//LevelOfDetail set into the tilesys class as private variables
	tilesys.setLevelOfDetail(levelOfdetail);

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

	// Set the AMQP filter
	set_filter(opts, s);

	proton::connection conn = c.connect(conn_url_);
	conn.open_receiver(addr_, proton::receiver_options().source(opts));
}

void 
AMQPClient::on_message(proton::delivery &d, proton::message &msg) {
	etsiDecoder::etsiDecodedData_t decodedData;

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

	// Decode the content of the message, using the decoder-module frontend class
    if(m_decodeFrontend.decodeEtsi(message_bin_buf, message_bin.size (), decodedData)!=ETSI_DECODER_OK) {
    	std::cerr << "Error! Cannot decode ETSI packet!" << std::endl;
    	return;
    }

    // If a CAM has been received, it should be used to update the internal in-memory database
	if(decodedData.type == etsiDecoder::ETSI_DECODED_CAM) {
		CAM_t *decoded_cam = (CAM_t *) decodedData.decoded_msg;
		double lat = decoded_cam->cam.camParameters.basicContainer.referencePosition.latitude/10000000.0;
		double lon = decoded_cam->cam.camParameters.basicContainer.referencePosition.longitude/10000000.0;
		uint32_t stationID = decoded_cam->header.stationID;

		// After getting the lat and lon values from the CAM, check if it is inside the S-LDM full coverage area,
		// using the areaFilter module (which can access the command line options, thus also the coverage area
		// specified by the user)
		if(m_areaFilter.isInside(lat,lon)==false) {
			return;
		}

		// Update the database
		ldmmap::vehicleData_t vehdata;
		ldmmap::LDMMap::LDMMap_error_t db_retval;
		vehdata.lon = lon;
		vehdata.lat = lat;
		vehdata.timestamp_us = get_timestamp_us();
		vehdata.elevation = decoded_cam->cam.camParameters.basicContainer.referencePosition.altitude.altitudeValue/100.0;
		vehdata.heading = decoded_cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.heading.headingValue/10.0;
		vehdata.speed_ms = decoded_cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.speed.speedValue/100.0;
		vehdata.gnTimestamp = decodedData.gnTimestamp;

		db_retval=m_db_ptr->insert(vehdata);

		if(db_retval!=ldmmap::LDMMap::LDMMAP_OK && db_retval!=ldmmap::LDMMap::LDMMAP_UPDATED) {
			std::cerr << "Warning! Insert on the database for vehicle " << (int) stationID << "failed!" << std::endl;
		}
	} else {
		std::cerr << "Warning! Only CAM messages are supported for the time being!" << std::endl;
		return;
	}
}