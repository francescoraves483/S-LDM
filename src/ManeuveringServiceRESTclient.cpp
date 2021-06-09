#include "ManeuveringServiceRESTclient.h"
#include "pthread.h"
#include "utils.h"

#define MAKE_NUM(val) web::json::value::number(val);
#define MAKE_STR(val) web::json::value::string(val);

using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous streams

void *RESTthread_callback (void *arg) {
	// Timer variables
	int clockFd;
	struct pollfd pollfddata;

	ManeuveringServiceRestClient *restObj = static_cast<ManeuveringServiceRestClient *>(arg);

	// JSON variables
	web::json::value json_response;

	web::json::value sldm_json;

	web::http::client::http_client *client = new web::http::client::http_client(restObj->getServerFullAddress());

	http::status_code status_code_ret;
	http::reason_phrase reason_phrase_ret;

	// Send 1 request every timems milliseconds until the server sends a reply containing the STOP in the JSON field "rsp_type"
	timer_fd_create(pollfddata, clockFd, restObj->getPeriodicInterval()*1000000.0);

	POLL_DEFINE_JUNK_VARIABLE();

	bool continue_flg = true;

	while(continue_flg == true) {
		if(poll(&pollfddata,1,0)>0) {

			POLL_CLEAR_EVENT(clockFd);

			sldm_json = restObj->make_SLDM_json(0);

			try {
				client->request(web::http::methods::POST, U("/"), sldm_json)
				.then([&status_code_ret,&reason_phrase_ret,&json_response](const web::http::http_response& response) {
					status_code_ret = response.status_code();
					reason_phrase_ret = response.reason_phrase();

					if(status_code_ret == web::http::status_codes::OK) {
						json_response = response.extract_json().get();
					}
				})
				.wait();

				std::cout << "Server returned code: " << status_code_ret << " (" << reason_phrase_ret << ")" << std::endl;
				std::cout << "JSON response type: " << json_response["rsp_type"].as_string() << " - generated at timestamp: " << json_response["rsp_time"].as_number().to_uint64() << std::endl;
				std::cout << "Estimated total RTT latency: " << (double)(get_timestamp_us()/1e3)-(double)(json_response["post_time"].as_number().to_uint64()/1e3) << " ms" << std::endl;

				if(json_response["rsp_type"].as_string() == "STOP") {
					// Close connection and delete the client object
					// Creating the HTTP client object with "new" and then deleting it seems to enable an early disconnection from the server
					delete client;

					continue_flg = false;
				}
			} catch(std::exception& excpt) {
				std::cerr << "An error occurred: " << excpt.what() << std::endl;

				if(strstr(excpt.what(),"Failed to connect") != NULL) {
					continue_flg = false;
				}
			}
		}
	}

	close(clockFd);

	restObj->setThreadRunningStatus(false);
	restObj->callNotifyFunction();

	// Destroy the object that generated this thread (in a sort of "self-destroy" mechanism)
	delete restObj;

	pthread_exit(NULL);
}

web::json::value ManeuveringServiceRestClient::make_SLDM_json(int eventID) {
	// Create a new JSON structure
	web::json::value sldm_json;

	// Returned vehicle data vector
	std::vector<ldmmap::LDMMap::returnedVehicleData_t> returnedvehs;

	sldm_json["generation_tstamp"] = web::json::value::number(get_timestamp_us());
	sldm_json["eventID"] = MAKE_NUM(eventID);
	sldm_json["event"] = MAKE_STR("CLC");
	sldm_json["reference_lat_deg"] = MAKE_NUM(m_lat);
	sldm_json["reference_lon_deg"] = MAKE_NUM(m_lon);
	sldm_json["reference_vehicle_ID"] = MAKE_NUM(m_refVehStationID);

	if(m_db_ptr->rangeSelect(m_range_m,m_lat,m_lon,returnedvehs)!=ldmmap::LDMMap::LDMMAP_OK) {
		sldm_json["error"] = MAKE_STR("ERROR");
	} else {
		sldm_json["error"] = MAKE_STR("OK");
	}

	int idx = 0;
	web::json::value vehicles = web::json::value::array();

	for(ldmmap::LDMMap::returnedVehicleData_t vehdata : returnedvehs) {
		vehicles[idx] = make_vehicle(vehdata.vehData.stationID,
			vehdata.vehData.lat,
			vehdata.vehData.lon,
			vehdata.vehData.exteriorLights.isAvailable() ? exteriorLights_bit_to_string(vehdata.vehData.exteriorLights.getData()) : "unavailable",
			vehdata.vehData.camTimestamp,
			vehdata.vehData.gnTimestamp,
			vehdata.vehData.vehicleLength,
			vehdata.vehData.vehicleWidth,
			vehdata.vehData.speed_ms,
			vehdata.phData);

		idx++;
	}

	sldm_json["vehicles"] = vehicles;

	return sldm_json;
}

web::json::value ManeuveringServiceRestClient::make_vehicle(uint64_t stationID, 
	double lat, 
	double lon, 
	std::string turnindicator,
	uint16_t CAM_tstamp,
	uint64_t GN_tstamp,
	ldmmap::OptionalDataItem<long> car_length_mm,
	ldmmap::OptionalDataItem<long> car_width_mm,
	double speed_ms,
	ldmmap::PHpoints *path_history
	) {

	web::json::value vehicle;

	vehicle["stationID"] = MAKE_NUM(stationID);
	vehicle["lat"] = MAKE_NUM(lat);
	vehicle["lon"] = MAKE_NUM(lon);
	vehicle["speed_ms"] = MAKE_NUM(speed_ms);
	vehicle["turnindicator"] = MAKE_STR(turnindicator);
	vehicle["CAM_tstamp"] = MAKE_NUM(CAM_tstamp);
	vehicle["GN_tstamp"] = MAKE_NUM(GN_tstamp);

	if(car_length_mm.isAvailable()) {
		vehicle["car_len_mm"] = MAKE_NUM(car_length_mm.getData());
	} else {
		vehicle["car_len_mm"] = MAKE_STR("unavailable");
	}

	if(car_width_mm.isAvailable()) {
		vehicle["car_wid_mm"] = MAKE_NUM(car_width_mm.getData());
	} else {
		vehicle["car_wid_mm"] = MAKE_STR("unavailable");
	}

	web::json::value PH_points_lat_json = web::json::value::array();
	web::json::value PH_points_lon_json = web::json::value::array();

	if(path_history!=nullptr) {
		int idx = 0;
		PHDATAITER_INITIALIZER(phdataiter);

		// Iterate over all the path history points, using the "iterate" method of the PHPoints object, 
		// storing the Path History points for each vehicle in the LDMMap database
		while(path_history->iterate(phdataiter,nullptr)!=ldmmap::PHpoints::PHP_TERMINATE_ITERATION) {
			PH_points_lat_json[idx] = MAKE_STR(std::to_string(phdataiter.data.lat));
			PH_points_lon_json[idx] = MAKE_STR(std::to_string(phdataiter.data.lon));

			idx++;
		}

		vehicle["PH_points_lat"] = PH_points_lat_json;
		vehicle["PH_points_lon"] = PH_points_lon_json;
	}

	return vehicle;
}

int ManeuveringServiceRestClient::startRESTthread(void) {
	pthread_t tid;

	// Return immediately as the REST thread is already running
	if(m_thread_running == true) {
		return -1;
	}

	pthread_attr_t tattr;

	// This thread is detached as we are not expected to pthread_join() on it
	pthread_attr_init(&tattr);
	pthread_attr_setdetachstate(&tattr,PTHREAD_CREATE_DETACHED);

	if(pthread_create(&tid,NULL,RESTthread_callback,(void *) this) == 0) {
		m_thread_running = true;
	}

	return static_cast<int>(tid);
}

std::string inline ManeuveringServiceRestClient::getServerFullAddress(void) {
	return m_srv_addr + ":" + std::to_string(m_port);
}

void inline ManeuveringServiceRestClient::callNotifyFunction(void) {
	if(m_notify_fcn!=nullptr) {
		m_notify_fcn(m_refVehStationID);
	}
}