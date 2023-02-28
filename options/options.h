/*
*	Options C module
*	
*	C module for easy and efficient command-line options management
*	The code is derived from LaTe, out project for an advanced latency measurement tool: https://github.com/francescoraves483/LaMP_LaTe
*
*	Copyright (C) 2019-2021 Francesco Raviglione (CNIT - Politecnico di Torino), Marco Malinverno (CNIT - Politecnico di Torino)
*	
*	This program is free software; you can redistribute it and/or
*	modify it under the terms of the GNU General Public License
*	as published by the Free Software Foundation; either version 2
*	of the License, or (at your option) any later version.
*	
*	This program is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*	
*	You should have received a copy of the GNU General Public License
*	along with this program; if not, write to the Free Software
*	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef OPTIONSMODULE_H_INCLUDED
#define OPTIONSMODULE_H_INCLUDED

#include <inttypes.h>
#include <stdbool.h>
#include <float.h>
#include "options_strings.h"

// The way in which this module defines the AMQP broker options for the (possible) additional AMQP Client relies on X-Macros
// The "CFG" macros below should contain the fields of the struct containing the options for the AMQP clients
// The fiels should be specified as (field name,field type). 'field type' can be either "options_string", which works in
// a similar way as C++ std::string (but it is needed here as this module is written in plain C), or "bool", for the time
// being. Adding additional types requires to edit AMQPCLIENT_FIELDS_ARRAY_FILL_DECLARE(name,type) and add additional "else if"
// branches checking for "strcmp(#type,"<new supported type here>")==0"
// The content of the broker_options_t structure is then automatically generated depending on what is defined below
// This macro also defines the content of the options structure for the main AMQP client
#define AMQPCLIENT_CONFIG_FIELDS \
	CFG(amqp_username,options_string) \
	CFG(amqp_password,options_string) \
	CFG(amqp_reconnect,bool) \
	CFG(amqp_allow_sasl,bool) \
	CFG(amqp_allow_insecure,bool) \
	CFG(broker_url,options_string) \
	CFG(broker_topic,options_string) \
	CFG(amqp_idle_timeout,long) \
	CFG(amqp_reconnect_after_local_timeout_expired,bool)

// The bool type is set to "_Bool" because "_Bool" is actually the true type name inside stdbool.h ("bool" is just an alias)
// This macro is used to automatically define several functions (used in options.c) to fill in the different fields of each
// element of the array (broker_options_t *amqp_broker_x) containing the options for the possible additional AMQP clients
// The generated functions will all accept the same arguments (optarg, i.e., the comma-separated string read from command line, 
// num_clients, i.e., total number of additional AMQP clients, broker_opts, i.e., a pointer to the AMQP clients options array)
// and will be named fill_AMQPClient_options_array_<field name>() (e.g., fill_AMQPClient_options_array_amqp_username())
// "*((bool *)&(broker_opts[i].name))" is indeed ugly and seems useless, but it is used as a trick to properly compile the generated
// code. Indeed, the preprocessor will generate the lines "broker_opts[i].name=true/false" for all the fiels, no matter whether
// they are boolean or options_string. Even if these lines will never be reached for the generated functions working on
// options_string fields, they need nevertheless to be compiled. Thus, we take the pointer to the field, we convert it
// to a boolean type pointer and then we dereference that pointer to get a "boolean". For real boolean fields the result 
// is the same as a direct assignment, while for options_string these lines will never be reached, but, at least, now
// everthing compiles since both the lhs and rhs values are now interpreted as "boolean".
#define AMQPCLIENT_FIELDS_ARRAY_FILL_DECLARE(name,type) \
static inline bool fill_AMQPClient_options_array_##name(char * optarg, int num_clients, broker_options_t *broker_opts) { \
	char *saveptr_strtok=NULL; \
	char *str_ptr; \
	unsigned int cnt=0; \
	for(int i=0;i<num_clients;i++) { \
		str_ptr=strtok_r(i==0 ? optarg : NULL,",",&saveptr_strtok); \
	 	if(str_ptr!=NULL) { \
	 		if(strcmp(#type,"options_string")==0) { \
	 			if(strcmp(str_ptr," ")==0) { \
	 				options_string_push((options_string *)&(broker_opts[i].name),""); \
	 			} else { \
		 			if(!options_string_push((options_string *)&(broker_opts[i].name),str_ptr)) { \
						fprintf(stderr,"Error in parsing the AMQP client option: %s. Client: %d.\n",#name,i); \
					} \
				} \
	 		} else if(strcmp(#type,"_Bool")==0) { \
	 			if(strcmp(str_ptr,"true")==0) { \
	 				*((bool *)&(broker_opts[i].name))=true; \
	 			} else if(strcmp(str_ptr,"false")==0) { \
	 				*((bool *)&(broker_opts[i].name))=false; \
	 			} else if(strcmp(str_ptr,"default")==0) { \
	 				*((bool *)&(broker_opts[i].name))=false; \
	 			} else { \
	 				fprintf(stderr,"Error in parsing the AMQP client option: %s. %.15s is not a valid boolean value (true,false,default). Client: %d.\n",#name,str_ptr,i); \
	 				return false; \
	 			} \
	 		} else if(strcmp(#type,"long")==0) { \
	 			char *sPtr; \
	 			errno=0;\
				*((long *)&(broker_opts[i].name))=strtol(optarg,&sPtr,10); \
				if(sPtr==optarg || errno) { \
					fprintf(stderr,"Error in parsing the AMQP client option: %s. Client: %d.\n",#name,i); \
				} \
	 		} else { \
	 			return false; \
	 		} \
	 		cnt++; \
	 	} else { \
			break; \
		} \
	} \
	if(cnt!=num_clients) { \
		return false; \
	} \
	return true; \
}

// Insert here the version string
#define VERSION_STR "S-LDM 1.2.10-beta" // 1.0.0 -> first (initial) cross-border version

#define DEFAULT_BROKER_URL "127.0.0.1:5672"
#define DEFAULT_BROKER_QUEUE "topic://5gcarmen.examples"

#define DEFAULT_MANEUVERING_SERVICE_REST_SRV_ADDR "http://localhost"
#define DEFAULT_MANEUVERING_SERVICE_REST_SRV_PORT 8000
#define DEFAULT_MANEUVERING_SERVICE_REST_PERIODICITY 1.0 // Specified in [s]

#define DEFAULT_VEHVIZ_NODEJS_UDP_ADDR "127.0.0.1"
#define DEFAULT_VEHVIZ_NODEJS_UDP_PORT 48110

#define DEFAULT_VEHVIZ_WEB_PORT 8080

#define DEFAULT_CONTEXT_RADIUS_METERS 150.0
#define MINIMUM_CONTEXT_RADIUS_METERS 10.0

// Default Vehicle Visualizer web-based GUI update rate, in seconds
#define DEFAULT_VEHVIZ_UPDATE_INTERVAL_SECONDS 0.5

// Maximum number of supported AMQP clients
#define MAX_ADDITIONAL_AMQP_CLIENTS 10

// Default name for gn-timestamp property to look for when there is no GN+BTP in the message
#define DEFAULT_GN_TIMESTAMP_PROPERTY "gn_ts"

// Default port for the on-demand JSON-over-TCP interface
#define DEFAULT_OD_JSON_OVER_TCP_INTERFACE_PORT 49000

// Valid options
// Any new option should be handled in the switch-case inside parse_options() and the corresponding char should be added to VALID_OPTS
// If an option accepts an additional argument, it is followed by ':'
#define VALID_OPTS "hvA:E:F:cU:Q:r:s:Z:z:w:L:u:p:RSIC:gOo:"

#define INIT_CODE 0xAE

#define INVALID_LONLAT -DBL_MAX

// Automatically generated thanks to AMQPCLIENT_CONFIG_FIELDS (X-Macros)
typedef struct broker_options {
	#define CFG(name,type) type name;
		AMQPCLIENT_CONFIG_FIELDS
	#undef CFG
} broker_options_t;

typedef struct options {
	// = INIT_CODE if 'struct options' has been initialized via options_initialize()
	uint8_t init_code;

	double min_lat;
	double min_lon;
	double max_lat;
	double max_lon;

	// Extended area parameters (additive factors to the min/max lat and lon values of the internal area)
	double ext_lat_factor;
	double ext_lon_factor;

	bool cross_border_trigger;

	options_string ms_rest_addr; // Maneuvering Service REST Server address (excluding the port number)
	long ms_rest_port; // Maneuvering Service REST Server port
	bool left_indicator_trg_enable; // When this option is set to 'true', the data transmission will be triggered also depending on the left turn indicator, other than considering the right one (which is the default behaviour when this option is 'false')
	bool ext_lights_hijack_enable;// When this options is set to 'true', the information for ext. lights is enabled to be extracted from a highFreqContainer field inside CAMs (to solve version incompatibilities issues), instead of using the correct container
	bool interop_hijack_enable;
	double ms_rest_periodicity; // Periodicity at which the REST data should be sent to other services (e.g., the Maneuvering Service)
	options_string gn_timestamp_property; // Name of the property to check in the amqp header for the gn-timestamp, when decoding messages without GN+BTP in their payloads

	options_string vehviz_nodejs_addr; // Advanced option: IPv4 address for the UDP connection to the Node.js server (excluding the port number)
	long vehviz_nodejs_port; // Advanced option: port number for the UDP connection to the Node.js server

	long vehviz_web_interface_port; // Port number for the Vehicle Visualizer web interface

	options_string logfile_name; // Name of the log file where performance data should be store (if specified)

	broker_options_t amqp_broker_one; // "one" because this the main AMQP client; all the other clients are set via "amqp_broker_x"

	// Additional AMQP clients options
	// ----------------------------------
	unsigned int num_amqp_x_enabled; // Number of additional AMQP clients which have been activated (>0 if any additional client has been activated)
	broker_options_t *amqp_broker_x; // Array of num_amqp_x_enabled elements (one element for the configuration of each additioanl AMQP client)
	// ----------------------------------

	double context_radius; // Radius (in m) of the "context" around a triggering vehicle (used for the time being only when sending the data to the Maneuvering Service through the simple indicatorTriggerManager)
	double vehviz_update_interval_sec; // Advanced option: modifies the update rate of the web-based GUI. Warning: decreasing this too much will affect performance! This value cannot be less than 0.05 s and more than 1 s.

	bool indicatorTrgMan_enabled; // 'true' if the turn indicator trigger manager is enabled (default option), 'false' otherwise

	bool ageCheck_enabled; // (-g option to set this to 'false') 'true' if an 'age check' on the received data should be performed before updating the database, 'false' otherwise. Default: 'true'.
	bool quadkFilter_enabled; // 'true' if the QuadKey filter is enabled (messages are pre-filtered by the AMQP broker depending on the Quadkey property), 'false' otherwise (default: 'true' - it must be explicitly disabled, if needed)

	bool od_json_interface_enabled; // Set to 'true' if the on-demand JSON-over-TCP interface is active on port od_json_interface_port, to 'false' otherwise
	long od_json_interface_port; // On-demand JSON-over-TCP interface port
} options_t;

void options_initialize(struct options *options);
unsigned int parse_options(int argc, char **argv, struct options *options);
void options_free(struct options *options);

#endif // OPTIONSMODULE_H_INCLUDED
