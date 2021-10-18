#include <proton/connection.hpp>
#include <proton/delivery.hpp>
#include <proton/message.hpp>
#include <proton/tracker.hpp>
#include <proton/connection_options.hpp>

#include "camrelayeramqp.h"
#include "sample_quad_final.h"

#include <iostream>
#include <unistd.h>

bool CAMrelayerAMQP::wait_sender_ready(void) {
	// We should not need any mutex lock, as m_sender_ready is defined as std::atomic<bool>
	// Waiting for the sender to become ready, i.e. waiting for m_sender_ready to become "true"
	while(!m_sender_ready);

	return m_sender_ready;
}

void CAMrelayerAMQP::sendCAM_AMQP(uint8_t *buffer, int bufsize,const double &lat, const double &lon, const int &lev,const uint32_t &gn_tst) {

    QuadKeys::QuadKeyTS tilesys;
	const double latitude = lat;
	const double longitude = lon;
	// const int levelofDetail = lev;
	const long gn_timestamp = gn_tst;


	proton::message CAM_msg;
	tilesys.setLevelOfDetail(18);
	std::string quadkeys = tilesys.LatLonToQuadKey(latitude, longitude);
	CAM_msg.properties().put("quadkeys", quadkeys);
//	CAM_msg.properties().put ("gn-timestamp",gn_timestamp);
	CAM_msg.properties().put (cr_arg_cl.m_gn_tst_prop_name,gn_timestamp);



//        std::cout<<"CAM: "<<std::endl;

//        for(int i=0;i<20;i++) {
//            printf("%02X",buffer[i]);
//          }
//        std::cout<<std::endl;

	// "Inject" the work of sending a new CAM with the current sender (m_sender)
	// Checking m_work_queue_ptr!=NULL just for additional safety
	if(m_work_queue_ptr!=NULL) {
		// Create the AMQP message from the buffer
		CAM_msg.body(proton::binary(buffer,buffer+bufsize));

		// Add the work of sending the CAM via m_sender
		m_work_queue_ptr->add([=]() {m_sender.send(CAM_msg);});
	}
}

CAMrelayerAMQP::CAMrelayerAMQP(const pthread_camrelayer_args_t camrelay_args) :
	cr_arg_cl(camrelay_args), m_work_queue_ptr(NULL), m_sender_ready(false) {}

CAMrelayerAMQP::CAMrelayerAMQP() :
	m_work_queue_ptr(NULL), m_sender_ready(false) {}

void CAMrelayerAMQP::set_args(const pthread_camrelayer_args_t camrelay_args) {
	cr_arg_cl=camrelay_args;
}

void CAMrelayerAMQP::on_container_start(proton::container& c) {
	proton::connection_options co;
	//c.connect(cr_arg_cl.m_broker_address,co.idle_timeout(proton::duration::FOREVER));
	c.connect(cr_arg_cl.m_broker_address,co.idle_timeout(proton::duration(1000)));
}

void CAMrelayerAMQP::on_connection_open(proton::connection& c) {
	c.open_sender(cr_arg_cl.m_queue_name);
}

void CAMrelayerAMQP::on_sender_open(proton::sender& protonsender) {
	m_sender=protonsender;

	// Get the work queue pointer out of the sender
	m_work_queue_ptr=&m_sender.work_queue();

	// Set "m_sender_ready" to true -> now the sender is ready and the application can safely call sendCAM_AMQP()
	m_sender_ready=true;
}

// This function basically does nothing -> the std::cout can be optionally enabled to print some debug information
void CAMrelayerAMQP::on_sendable(proton::sender &s) {

    //std::cout<<"Credit left: "<<s.credit()<<std::endl;

}

// This function basically does nothing other than printing "on_message" -> you can enable the "on_message" printing for debug purposes by decommenting the content of the function
void CAMrelayerAMQP::on_message(proton::delivery &dlvr, proton::message &msg) {
	//std::cout<<"on_message: "<<std::endl;
}
