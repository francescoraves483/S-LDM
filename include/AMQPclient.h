#ifndef SLDM_AMQP_CLIENT_H
#define SLDM_AMQP_CLIENT_H

#include <proton/connection.hpp>
#include <proton/connection_options.hpp>
#include <proton/container.hpp>
#include <proton/message.hpp>
#include <proton/message_id.hpp>
#include <proton/messaging_handler.hpp>
#include <proton/tracker.hpp>
#include <proton/types.hpp>
#include <proton/source_options.hpp>
#include <proton/receiver_options.hpp>

#include <memory>
#include <atomic>

#include "etsiDecoderFrontend.h"
#include "areaFilter.h"
#include "LDMmap.h"
#include "utils.h"
#include "triggerManager.h"

class AMQPClient : public proton::messaging_handler {
	private:
		std::string conn_url_;
		std::string addr_;
		double max_latitude;
		double max_longitude;
		double min_latitude;
		double min_longitude;

		bool m_printMsg; // If 'true' each received message will be printed (default: 'false' - enable only for debugging purposes)

		etsiDecoder::decoderFrontend m_decodeFrontend;
		areaFilter m_areaFilter;
		struct options *m_opts_ptr;
		ldmmap::LDMMap *m_db_ptr;
		indicatorTriggerManager *m_indicatorTrgMan_ptr;
		bool m_indicatorTrgMan_enabled;

		std::string m_logfile_name;
		FILE *m_logfile_file;

		std::string m_username;
		std::string m_password;
		bool m_reconnect;
		bool m_allow_sasl;
		bool m_allow_insecure;
		long m_idle_timeout_ms;

		std::string m_client_id;

		std::string m_quadKey_filter="";

		std::atomic<proton::container *> m_cont;
	public:
		AMQPClient(const std::string &u,const std::string &a,const double &latmin,const double &latmax,const double &lonmin, const double &lonmax, struct options *opts_ptr, ldmmap::LDMMap *db_ptr, std::string logfile_name) :
		conn_url_(u), addr_(a), max_latitude(latmax), max_longitude(lonmax), min_latitude(latmin), min_longitude(lonmin), m_opts_ptr(opts_ptr), m_db_ptr(db_ptr), m_logfile_name(logfile_name), m_quadKey_filter("") {
			m_printMsg=false;
			m_areaFilter.setOptions(m_opts_ptr);
			m_indicatorTrgMan_enabled=false;
			m_logfile_file=nullptr;
			m_reconnect=false;
			m_allow_sasl=false;
			m_allow_insecure=false;
			m_client_id="unset";
			m_cont=nullptr;
			m_idle_timeout_ms=-1;
		}

		AMQPClient(const std::string &u,const std::string &a,const double &latmin,const double &latmax,const double &lonmin, const double &lonmax, struct options *opts_ptr, ldmmap::LDMMap *db_ptr) :
		conn_url_(u), addr_(a), max_latitude(latmax), max_longitude(lonmax), min_latitude(latmin), min_longitude(lonmin), m_opts_ptr(opts_ptr), m_db_ptr(db_ptr), m_quadKey_filter("") {
			m_printMsg=false;
			m_areaFilter.setOptions(m_opts_ptr);
			m_indicatorTrgMan_enabled=false;
			m_logfile_name = "";
			m_logfile_file=nullptr;
			m_reconnect=false;
			m_allow_sasl=false;
			m_allow_insecure=false;
			m_client_id="unset";
			m_cont=nullptr;
			m_idle_timeout_ms=-1;
		}

		void setIndicatorTriggerManager(indicatorTriggerManager *indicatorTrgMan_ptr) {
			m_indicatorTrgMan_ptr=indicatorTrgMan_ptr;
			m_indicatorTrgMan_enabled=true;
		}

		void setUsername(std::string username) {
			m_username=username;
		}

		void setPassword(std::string password) {
			m_password=password;
		}

		void setConnectionOptions(bool allow_sasl,bool allow_insecure,bool reconnect = false) {
			m_allow_sasl=allow_sasl;
			m_allow_insecure=allow_insecure;
			m_reconnect=reconnect;
		}

		void setCredentials(std::string username,std::string password,bool allow_sasl,bool allow_insecure,bool reconnect = false) {
			m_username=username;
			m_password=password;
			m_allow_sasl=allow_sasl;
			m_allow_insecure=allow_insecure;
			m_reconnect=reconnect;
		}

		void setIdleTimeout(long idle_timeout_ms) {
			m_idle_timeout_ms=idle_timeout_ms;
		}

		void setClientID(std::string id) {
			m_client_id=id;
		}

		void setFilter(std::string &filter) {
			m_quadKey_filter=filter;
		}
		
		inline ldmmap::OptionalDataItem<uint8_t> manage_LowfreqContainer(CAM_t *decoded_cam,uint32_t stationID);

		void on_container_start(proton::container &c) override;
		void on_connection_open(proton::connection &conn) override;
		void on_message(proton::delivery &d, proton::message &msg) override;
		void on_container_stop(proton::container &c) override;
		void on_connection_close(proton::connection &conn) override;

		// This function can be used to force the termination of the event loop of the current client from outside the AMQPClient object
		// "m_cont" is an atomic pointer to the proton::container which is set as soon as on_container_start() is called
		void force_container_stop() {
			if(m_cont!=nullptr) {
				m_cont.load()->stop();
				m_cont=nullptr;
			}
		}

		void setPrintMsg(bool printMsgEnable) {m_printMsg = printMsgEnable;}
};

#endif // SLDM_AMQP_CLIENT_H
