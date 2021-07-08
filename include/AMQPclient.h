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

#include "etsiDecoderFrontend.h"
#include "areaFilter.h"
#include "LDMmap.h"
#include "utils.h"
#include "triggerManager.h"

class AMQPClient : public proton::messaging_handler {
	private:
		std::string url;
		std::string user;
		std::string password;

		std::string conn_url_;
		std::string addr_;
		double max_latitude;
		double max_longitude;
		double min_latitude;
		double min_longitude;
		int levelOfdetail;

		bool m_printMsg; // If 'true' each received message will be printed (default: 'false' - enable only for debugging purposes)

		etsiDecoder::decoderFrontend m_decodeFrontend;
		areaFilter m_areaFilter;
		struct options *m_opts_ptr;
		ldmmap::LDMMap *m_db_ptr;
		indicatorTriggerManager m_indicatorTrgMan;
		bool m_indicatorTrgMan_enabled;

		std::string m_logfile_name;
		FILE *m_logfile_file;
	public:
		AMQPClient(const std::string &u,const std::string &a,const double &latmin,const double &latmax,const double &lonmin, const double &lonmax, const int &lev, struct options *opts_ptr, ldmmap::LDMMap *db_ptr, std::string logfile_name) :
      	conn_url_(u), addr_(a), max_latitude(latmax), max_longitude(lonmax), min_latitude(latmin), min_longitude(lonmin), levelOfdetail(lev), m_opts_ptr(opts_ptr), m_db_ptr(db_ptr), m_indicatorTrgMan(db_ptr,opts_ptr), m_logfile_name(logfile_name) {
      		m_printMsg=false;
      		m_areaFilter.setOptions(m_opts_ptr);
      		m_indicatorTrgMan_enabled=false;
      		m_logfile_file=nullptr;
      	}

      	AMQPClient(const std::string &u,const std::string &a,const double &latmin,const double &latmax,const double &lonmin, const double &lonmax, const int &lev, struct options *opts_ptr, ldmmap::LDMMap *db_ptr) :
      	conn_url_(u), addr_(a), max_latitude(latmax), max_longitude(lonmax), min_latitude(latmin), min_longitude(lonmin), levelOfdetail(lev), m_opts_ptr(opts_ptr), m_db_ptr(db_ptr), m_indicatorTrgMan(db_ptr,opts_ptr) {
      		m_printMsg=false;
      		m_areaFilter.setOptions(m_opts_ptr);
      		m_indicatorTrgMan_enabled=false;
      		m_logfile_name = "";
      		m_logfile_file=nullptr;
      	}

      	void setIndicatorTriggerManager(bool enabled) {m_indicatorTrgMan_enabled=enabled;}
		
		void on_container_start(proton::container &c) override;
		void on_connection_open(proton::connection &conn) override;
		void on_message(proton::delivery &d, proton::message &msg) override;
		void on_container_stop(proton::container &c) override;

		void setPrintMsg(bool printMsgEnable) {m_printMsg = printMsgEnable;}
};

#endif // SLDM_AMQP_CLIENT_H
