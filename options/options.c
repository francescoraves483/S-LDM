#include "options.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

// Define the functions to fill in the additional AMQP clients (if any) options array
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#define CFG(name,type) AMQPCLIENT_FIELDS_ARRAY_FILL_DECLARE(name,type)
	AMQPCLIENT_CONFIG_FIELDS
#undef CFG
#pragma GCC diagnostic pop

// Names for the long options
#define LONGOPT_h "help"
#define LONGOPT_v "version"
#define LONGOPT_A "area"
#define LONGOPT_E "ext-area-lat-factor"
#define LONGOPT_F "ext-area-lon-factor"
#define LONGOPT_c "cross-border-trigger"
#define LONGOPT_U "broker-url"
#define LONGOPT_Q "broker-queue"
#define LONGOPT_r "ms-rest-address"
#define LONGOPT_s "ms-rest-port"
#define LONGOPT_Z "vehviz-nodejs-address"
#define LONGOPT_z "vehviz-nodejs-port"
#define LONGOPT_w "vehviz-web-port"
#define LONGOPT_L "log-file-name"
#define LONGOPT_C "context-radius"
#define LONGOPT_g "disable-age-check"
#define LONGOPT_O "enable-on-demand-requests"
#define LONGOPT_o "set-on-demand-json-port"

// AMQP broker (main)
#define LONGOPT_u "amqp-main-username"
#define LONGOPT_p "amqp-main-password"
#define LONGOPT_R "amqp-main-reconnect"
#define LONGOPT_S "amqp-main-allow-sasl"
#define LONGOPT_I "amqp-main-allow-plain"

// Long-only options
#define LONGOPT_vehviz_update_interval_sec "vehviz-update-interval"
#define LONGOPT_indicator_trgman_disable "indicator-trgman-disable"
#define LONGOPT_disable_quadkey_filter "disable-quadkey-filter"
#define LONGOPT_amqp_main_idle_timeout "amqp-main-idle-timeout"
#define LONGOPT_amqp_main_reconn_local_timeout_exp "amqp-main-reconn-local-timeout-exp"
#define LONGOPT_enable_left_indicator_trigger "enable-left-indicator-trigger"
#define LONGOPT_ms_rest_periodicity "ms-rest-periodicity"
#define LONGOPT_gn_timestamp_property "gn-timestamp-property"
#define LONGOPT_enable_ext_lights_hijack "enable-ext-lights-hijack"
#define LONGOPT_enable_interop_hijack "enable-interop-hijack"
// The corresponding "val"s are used internally and they should be set as sequential integers starting from 256 (the range 320-399 should not be used as it is reserved to the AMQP broker long options)
#define LONGOPT_vehviz_update_interval_sec_val 256
#define LONGOPT_indicator_trgman_disable_val 257
#define LONGOPT_disable_quadkey_filter_val 258
#define LONGOPT_amqp_main_idle_timeout_val 259
#define LONGOPT_amqp_main_reconn_local_timeout_exp_val 260
#define LONGOPT_enable_left_indicator_trigger_val 261
#define LONGOPT_ms_rest_periodicity_val 262
#define LONGOPT_gn_timestamp_property_val 263
#define LONGOPT_enable_ext_lights_hijack_val 264
#define LONGOPT_enable_interop_hijack_val 265

// AMQP broker (additional)
#define LONGOPT_amqp_enable_additionals "amqp-enable-additionals"
#define LONGOPT_amqp_additionals_url "amqp-additionals-url"
#define LONGOPT_amqp_additionals_queue "amqp-additionals-queue"
#define LONGOPT_amqp_additionals_username "amqp-additionals-username"
#define LONGOPT_amqp_additionals_password "amqp-additionals-password"
#define LONGOPT_amqp_additionals_reconnect "amqp-additionals-reconnect"
#define LONGOPT_amqp_additionals_allow_sasl "amqp-additionals-allow-sasl"
#define LONGOPT_amqp_additionals_allow_plain "amqp-additionals-allow-plain"
#define LONGOPT_amqp_additionals_idle_timeout "amqp-additionals-idle-timeout"
#define LONGOPT_amqp_additionals_reconn_local_timeout_exp "amqp-additionals-reconn-local-timeout-exp"
#define LONGOPT_amqp_enable_additionals_val 320
#define LONGOPT_amqp_additionals_url_val 321
#define LONGOPT_amqp_additionals_queue_val 322
#define LONGOPT_amqp_additionals_username_val 323
#define LONGOPT_amqp_additionals_password_val 324
#define LONGOPT_amqp_additionals_reconnect_val 325
#define LONGOPT_amqp_additionals_allow_sasl_val 326
#define LONGOPT_amqp_additionals_allow_plain_val 327
#define LONGOPT_amqp_additionals_idle_timeout_val 328
#define LONGOPT_amqp_additionals_reconn_local_timeout_exp_val 329


#define LONGOPT_STR_CONSTRUCTOR(LONGOPT_STR) "  --"LONGOPT_STR"\n"

#define STRINGIFY(value) STR(value)
#define STR(value) #value

// Long options "struct option" array for getopt_long
static const struct option long_opts[]={
	{LONGOPT_h,				no_argument, 		NULL, 'h'},
	{LONGOPT_v,				no_argument, 		NULL, 'v'},
	{LONGOPT_A,				required_argument,	NULL, 'A'},
	{LONGOPT_E,				required_argument,	NULL, 'E'},
	{LONGOPT_F,				required_argument,	NULL, 'F'},
	{LONGOPT_c,				no_argument,		NULL, 'c'},
	{LONGOPT_U,				required_argument,	NULL, 'U'},
	{LONGOPT_Q,				required_argument,	NULL, 'Q'},
	{LONGOPT_r,				required_argument,	NULL, 'r'},
	{LONGOPT_s,				required_argument,	NULL, 's'},
	{LONGOPT_Z,				required_argument,	NULL, 'Z'},
	{LONGOPT_z,				required_argument,	NULL, 'z'},
	{LONGOPT_w,				required_argument,	NULL, 'w'},
	{LONGOPT_L,				required_argument,	NULL, 'L'},
	{LONGOPT_u,				required_argument,	NULL, 'u'},
	{LONGOPT_p,				required_argument,	NULL, 'p'},
	{LONGOPT_R,				no_argument,		NULL, 'R'},
	{LONGOPT_S,				no_argument,		NULL, 'S'},
	{LONGOPT_I,				no_argument,		NULL, 'I'},
	{LONGOPT_C,				required_argument,	NULL, 'C'},
	{LONGOPT_g,				no_argument,		NULL, 'g'},
	{LONGOPT_O,				no_argument,		NULL, 'O'},
	{LONGOPT_o,				required_argument,	NULL, 'o'},

	{LONGOPT_vehviz_update_interval_sec,			required_argument,	NULL, LONGOPT_vehviz_update_interval_sec_val},
	{LONGOPT_indicator_trgman_disable,				no_argument,		NULL, LONGOPT_indicator_trgman_disable_val},
	{LONGOPT_disable_quadkey_filter,				no_argument,		NULL, LONGOPT_disable_quadkey_filter_val},
	{LONGOPT_amqp_main_idle_timeout,				required_argument,	NULL, LONGOPT_amqp_main_idle_timeout_val},
	{LONGOPT_amqp_main_reconn_local_timeout_exp,	no_argument,		NULL, LONGOPT_amqp_main_reconn_local_timeout_exp_val},
	{LONGOPT_enable_left_indicator_trigger,			no_argument,		NULL, LONGOPT_enable_left_indicator_trigger_val},
	{LONGOPT_ms_rest_periodicity,					required_argument,	NULL, LONGOPT_ms_rest_periodicity_val},
	{LONGOPT_gn_timestamp_property,					required_argument,	NULL, LONGOPT_gn_timestamp_property_val},
	{LONGOPT_enable_ext_lights_hijack,			no_argument,		NULL, LONGOPT_enable_ext_lights_hijack_val},
	{LONGOPT_enable_interop_hijack,			no_argument,		NULL, LONGOPT_enable_interop_hijack_val},

	// Additional AMQP clients options
	{LONGOPT_amqp_enable_additionals,					required_argument,		NULL, LONGOPT_amqp_enable_additionals_val},
	{LONGOPT_amqp_additionals_url,						required_argument,		NULL, LONGOPT_amqp_additionals_url_val},
	{LONGOPT_amqp_additionals_queue,					required_argument,		NULL, LONGOPT_amqp_additionals_queue_val},
	{LONGOPT_amqp_additionals_username,					required_argument,		NULL, LONGOPT_amqp_additionals_username_val},
	{LONGOPT_amqp_additionals_password,					required_argument,		NULL, LONGOPT_amqp_additionals_password_val},
	{LONGOPT_amqp_additionals_reconnect,				required_argument,		NULL, LONGOPT_amqp_additionals_reconnect_val},
	{LONGOPT_amqp_additionals_allow_sasl,				required_argument,		NULL, LONGOPT_amqp_additionals_allow_sasl_val},
	{LONGOPT_amqp_additionals_allow_plain,				required_argument,		NULL, LONGOPT_amqp_additionals_allow_plain_val},
	{LONGOPT_amqp_additionals_idle_timeout,				required_argument,		NULL, LONGOPT_amqp_additionals_idle_timeout_val},
	{LONGOPT_amqp_additionals_reconn_local_timeout_exp,	required_argument,		NULL, LONGOPT_amqp_additionals_reconn_local_timeout_exp_val},
	

	{NULL, 0, NULL, 0}
};

// Option strings: define here the description for each option to be then included inside print_long_info()
#define OPT_A_description \
	LONGOPT_STR_CONSTRUCTOR(LONGOPT_A) \
	"  -A <area coordinates>: set the internal area covered by the S-LDM. The string specified\n" \
	"\t  after this options should refer two 'vertices' of the area covered by the S-LDM (i.e.\n" \
	"\t  the point with the minimum latitude and longitude values and the one with the maximum\n" \
	"\t  latitude and logitude values).\n" \
	"\t  The string format should be: <min lat>:<min lon>-<max lat>:<max lon>\n"

#define OPT_E_description \
	LONGOPT_STR_CONSTRUCTOR(LONGOPT_E) \
	"  -E <latitude factor in degrees>: set the additive factor to add to the latitude values\n" \
	"\t  defining the internal area, in order to define the external S-LDM area, superimposed\n" \
	"\t  with the internal area of another S-LDM instance. Default value: 0 degrees (no external\n" \
	"\t  area for what latitude values are concerned).\n"

#define OPT_F_description \
	LONGOPT_STR_CONSTRUCTOR(LONGOPT_F) \
	"  -F <longitude factor in degrees>: set the additive factor to add to the longitude values\n" \
	"\t  defining the internal area, in order to define the external S-LDM area, superimposed\n" \
	"\t  with the internal area of another S-LDM instance. Default value: 0 degrees (no external\n" \
	"\t  area for what longitude values are concerned).\n"

#define OPT_c_description \
	LONGOPT_STR_CONSTRUCTOR(LONGOPT_c) \
	"  -c: set the cross border trigger mode. In this mode, two S-LDM instances with the -c\n" \
	"\t  option and covering a border area will both send their data to other services even\n" \
	"\t  if the triggering vehicle is located in their extended areas (not inside their internal\n" \
	"\t  area). This mode is disabled by default.\n"

#define OPT_U_description \
	LONGOPT_STR_CONSTRUCTOR(LONGOPT_U) \
	"  -U: set the AMQP broker URL, including the port. Default: ("DEFAULT_BROKER_URL").\n"

#define OPT_Q_description \
	LONGOPT_STR_CONSTRUCTOR(LONGOPT_Q) \
	"  -Q: set the AMQP queue or topic to be used. Default: ("DEFAULT_BROKER_QUEUE").\n"

#define OPT_r_description \
	LONGOPT_STR_CONSTRUCTOR(LONGOPT_r) \
	"  -r: set the Maneuvering Service REST server address, excluding the port.\n" \
	"\t  Default: ("DEFAULT_MANEUVERING_SERVICE_REST_SRV_ADDR").\n"

#define OPT_s_description \
	LONGOPT_STR_CONSTRUCTOR(LONGOPT_s) \
	"  -s: set the Maneuvering Service REST server port.\n" \
	"\t  Default: ("STRINGIFY(DEFAULT_MANEUVERING_SERVICE_REST_SRV_PORT)").\n"

#define OPT_Z_description \
	LONGOPT_STR_CONSTRUCTOR(LONGOPT_Z) \
	"  -Z <address>: advanced option: set the IPv4 address for the UDP connection to the Vehicle Visualizer\n" \
	"\t  Node.js server (excluding the port number).\n" \
	"\t  This is the address without port number. Default: ("DEFAULT_VEHVIZ_NODEJS_UDP_ADDR").\n"

#define OPT_z_description \
	LONGOPT_STR_CONSTRUCTOR(LONGOPT_z) \
	"  -z <port>: advanced option: set the port number for the UDP connection to the Vehicle Visualizer\n" \
	"\t  Node.js server.\n" \
	"\t  Default: ("STRINGIFY(DEFAULT_VEHVIZ_NODEJS_UDP_PORT)").\n"

#define OPT_w_description \
	LONGOPT_STR_CONSTRUCTOR(LONGOPT_w) \
	"  -w: set the port at which the web interface of the Vehicle Visualizer will be available.\n" \
	"\t  Default: ("STRINGIFY(DEFAULT_VEHVIZ_WEB_PORT)").\n"

#define OPT_L_description \
	LONGOPT_STR_CONSTRUCTOR(LONGOPT_L) \
	"  -L: enable log mode and set the name of the textual file where the data will be saved.\n" \
	"\t  'stdout' can be specified to output the log data on the screen. Default: (disabled).\n"

#define OPT_C_description \
	LONGOPT_STR_CONSTRUCTOR(LONGOPT_C) \
	"  -C <radius in m>: set the radius, in m, of the context around the triggering vehicle.\n" \
	"\t  The \"context\" contains the information of all the vehicles within a certain radius around\n" \
	"\t  the triggering vehicle.\n" \
	"\t  This option currently affects only the data transmission to the Maneuvering Service.\n" \
	"\t  Default: (150).\n"

#define OPT_g_description \
	LONGOPT_STR_CONSTRUCTOR(LONGOPT_g) \
	"  -g: disable data age check when updating the database. When this option is active, the latest.\n" \
	"\t  received data is always saved to the database, for each object, no matter if the timestamps\n" \
	"\t  are older than the one of the data already stored.\n" \
	"\t  When this option is not specified, GeoNetworking timestamps are used to understand if the\n" \
	"\t  received data is up-to-date and should be saved to the database or not.\n"

#define OPT_u_description \
	LONGOPT_STR_CONSTRUCTOR(LONGOPT_u) \
	"  -u <username>: set the username (if required) for the main AMQP client.\n"

#define OPT_p_description \
	LONGOPT_STR_CONSTRUCTOR(LONGOPT_p) \
	"  -p <password>: set the password (if required) for the main AMQP client.\n"

#define OPT_R_description \
	LONGOPT_STR_CONSTRUCTOR(LONGOPT_R) \
	"  -R: enable the automatic reconnection, after a connection loss, for the main AMQP client.\n"

#define OPT_S_description \
	LONGOPT_STR_CONSTRUCTOR(LONGOPT_S) \
	"  -S: enable the SASL authentication for the main AMQP client.\n"

#define OPT_I_description \
	LONGOPT_STR_CONSTRUCTOR(LONGOPT_I) \
	"  -I: enable the PLAIN authentication for the main AMQP client.\n"

#define OPT_O_description \
	LONGOPT_STR_CONSTRUCTOR(LONGOPT_O) \
	"  -O: enable the on-demand JSON-over-TCP interface for data retrieval from the S-LDM database.\n" \
	"\t  The S-LDM will expose a JSON-over-TCP server accepting TCP packets with a JSON containing the following fields:\n" \
	"\t  - \"lat\": latitude of the center of the target area\n" \
	"\t  - \"lon\": longitude of the center of the target area\n" \
	"\t  - \"range\": radius in meters of the target area aroud the center\n" \
	"\t  If the range is not specified, a default value of 300 m is automatically used.\n" \
	"\t  Upon reception of a proper request (i.e., TCP packet with a proper JSON) from a client, the S-LDM will return the\n" \
	"\t  list of vehicles and other objects stored in the database and located inside the target area.\n" \
	"\t  The on-demand JSON-over-TCP interface is disabled by default.\n"

#define OPT_o_description \
	LONGOPT_STR_CONSTRUCTOR(LONGOPT_o) \
	"  -o <port>: set the port for the on-demand JSON-over-TCP interface server. Default: ("STRINGIFY(DEFAULT_OD_JSON_OVER_TCP_INTERFACE_PORT)")\n"

#define OPT_vehviz_update_interval_sec_description \
	"  --"LONGOPT_vehviz_update_interval_sec" <interval in seconds>: advanced option: this option can be used to\n" \
	"\t  modify the update rate of the web-based GUI. \n" \
	"\t  Warning: decreasing too much this value will affect the S-LDM database performance!\n" \
	"\t  This value cannot be less than 0.05 s and more than 1 s. Default: ("STRINGIFY(DEFAULT_VEHVIZ_UPDATE_INTERVAL_SECONDS)")\n"

#define OPT_indicator_trgman_disable_description \
	"  --"LONGOPT_indicator_trgman_disable": advanced option: disable the indicator trigger manager and\n" \
	"\t  the transmission of data to the Maneuvering Service via the REST API.\n"

#define OPT_disable_quadkey_filter_description \
	"  --"LONGOPT_disable_quadkey_filter": disable the QuadKey filter to avoid pre-filtering of messages at the AMQP broker\n" \
	"\t  level. When this option is specified, all the messages will be received and possibly discarded at the Area Filter\n" \
	"\t  module, no matter the position of the transmitting vehicle. Typically, this option should not be specified, unless\n" \
	"\t  for some reason the vehicles are not inserting the \"quadkeys\" property inside their AMQP messages. If this is the\n" \
	"\t  case, you can use this option to enable the reception of all the messages without the aforementioned property.\n" \
	"\t  However, using this option may affect the overall performance of the S-LDM.\n"

#define OPT_amqp_main_idle_timeout \
	"  --"LONGOPT_amqp_main_idle_timeout": set the main AMQP broker idle timout value (any value < 0 means that the default\n" \
	"\t  Qpid Proton settings are used, i.e., the idle timeout is not explicitely set). Setting this to 0 will disable the\n" \
	"\t  the idle timeout (i.e., set it to FOREVER).\n"

#define OPT_amqp_main_reconn_local_timeout_exp \
	"  --"LONGOPT_amqp_main_reconn_local_timeout_exp": enable the automatic reconnection to the AMQP broker after a local\n" \
	"\t  idle timeout error occurs (amqp:resource-limit-exceeded: local-idle-timeout expired).\n" \
	"\t  Setting this option may help solving some issues with certain AMQP broker which seems to randomly cause this\n" \
	"\t  error (default: not enabled - i.e., no automatic reconnection occurs).\n"

#define OPT_brokers_enable_description \
	"  --"LONGOPT_amqp_enable_additionals" <number of additional clients>: this option can be used to enable the subscription to additional brokers,\n" \
	"\t  other than the main one. Up to 9 additional AMQP clients can be used, for the time being.\n" \
	"\t  The additional clients can then be configured with:\n" \
	"\t  "LONGOPT_STR_CONSTRUCTOR(LONGOPT_amqp_additionals_url) "" \
	"\t  "LONGOPT_STR_CONSTRUCTOR(LONGOPT_amqp_additionals_queue) "" \
	"\t  "LONGOPT_STR_CONSTRUCTOR(LONGOPT_amqp_additionals_username) "" \
	"\t  "LONGOPT_STR_CONSTRUCTOR(LONGOPT_amqp_additionals_password) "" \
	"\t  "LONGOPT_STR_CONSTRUCTOR(LONGOPT_amqp_additionals_reconnect) "" \
	"\t  "LONGOPT_STR_CONSTRUCTOR(LONGOPT_amqp_additionals_allow_sasl) "" \
	"\t  "LONGOPT_STR_CONSTRUCTOR(LONGOPT_amqp_additionals_allow_plain) "" \
	"\t  "LONGOPT_STR_CONSTRUCTOR(LONGOPT_amqp_additionals_idle_timeout) "" \
	"\t  "LONGOPT_STR_CONSTRUCTOR(LONGOPT_amqp_additionals_reconn_local_timeout_exp) "" \
	"\t  All these options work just like the corresponding ones for the main AMQP client, but they all accept a comma-separated\n" \
	"\t  list, where each entry corresponds, respectively, to the first client, then to the second, then to the third, and\n" \
	"\t  so on. The number of elements in each list should be equal to <number of additional clients>.\n" \
	"\t  The last three options are optional and, if not specified, the corresponding default values (all set to 'false'/\n" \
	"\t  'disabled') are set for all the additional clients. The username and password options, instead, if not specified,\n" \
	"\t  will not set the credentials for all the additional clients. Instead, if some clients need the credentials and some other\n" \
	"\t  don't, you should specify a list with some empty entries (as a single empty space), such as: \"myuser1, , ,myuser2\".\n" \
	"\t  Example with just one additional AMQP client:\n" \
	"\t  --amqp-enable-additionals 1 --amqp-additionals-url 127.0.0.1:5673 --amqp-additionals-queue topic://yourtopic.name\n"

#define OPT_enable_left_indicator_trigger \
	"  --"LONGOPT_enable_left_indicator_trigger": when this option is specified, the indicator trigger manager will trigger\n" \
	"\t  the data transmission via REST even when the left turn indicator is turned on by a vehicle. By default, only the right\n" \
	"\t  turn indicator is considered.\n"

#define OPT_ms_rest_periodicity \
	"  --"LONGOPT_ms_rest_periodicity " <value in seconds, can be floating point>: this option can be used to set the\n" \
	"\t  periodicity at which the data is sent via REST interface when a triggering condition is detected (default: 1 s).\n"

#define OPT_gn_timestamp_property \
	"  --"LONGOPT_gn_timestamp_property ":set the name of the gn-timestamp property to look for in the amqp header when the\n" \
	"\t  messages do not contain GN+BTP headers in the payload. Default: ("DEFAULT_GN_TIMESTAMP_PROPERTY").\n"

#define OPT_enable_ext_lights_hijack \
	"  --"LONGOPT_enable_ext_lights_hijack": when this options is set to 'true', the information for ext. lights is enabled\n" \
	"\t  to be extracted from a highFreqContainer field inside CAMs (to solve version incompatibilities issues with the lowFreqContainer),\n" \
	"\t  instead of using the correct container. Normal users should never use this advanced option, unless they know very\n" \
	"\t  well what they are doing."

#define OPT_enable_interop_hijack \
	"  --"LONGOPT_enable_interop_hijack": this is an advanced option not supposed to be enabled by the user. When this option is set\n" \
	"\t  to true, the data about specific vehicles is sent via REST with stationType=100 for easy filtering. Furthermore, if set\n" \
	"\t  to true, these specific vehicles won't trigger data transmission via REST when a turn indicator is on.\n"

static void print_long_info(char *argv0) {
	fprintf(stdout,"\nUsage: %s [-A S-LDM coverage internal area] [options]\n"
		"%s [-h | --"LONGOPT_h"]: print help and show options\n"
		"%s [-v | --"LONGOPT_v"]: print version information\n\n"

		"[options]:\n"
		OPT_A_description
		OPT_E_description
		OPT_F_description
		OPT_c_description
		OPT_U_description
		OPT_Q_description
		OPT_r_description
		OPT_s_description
		OPT_Z_description
		OPT_z_description
		OPT_w_description
		OPT_L_description
		OPT_C_description
		OPT_g_description
		OPT_u_description
		OPT_p_description
		OPT_R_description
		OPT_S_description
		OPT_I_description
		OPT_O_description
		OPT_o_description
		OPT_vehviz_update_interval_sec_description
		OPT_indicator_trgman_disable_description
		OPT_disable_quadkey_filter_description
		OPT_enable_left_indicator_trigger
		OPT_enable_ext_lights_hijack
		OPT_enable_interop_hijack
		OPT_ms_rest_periodicity
		OPT_gn_timestamp_property
		OPT_amqp_main_idle_timeout
		OPT_amqp_main_reconn_local_timeout_exp
		OPT_brokers_enable_description
		,
		argv0,argv0,argv0);

	exit(EXIT_SUCCESS);
}

static void print_short_info_err(struct options *options,char *argv0) {
	options_free(options);

	fprintf(stdout,"\nUsage: %s [-A S-LDM coverage internal area] [options]\n"
		"%s [-h | --"LONGOPT_h"]: print help and show options\n"
		"%s [-v | --"LONGOPT_v"]: print version information\n\n"
		,
		argv0,argv0,argv0);

	exit(EXIT_SUCCESS);
}

static void broker_options_inizialize(broker_options_t *broker_options) {
	broker_options->amqp_username=options_string_declare();
	broker_options->amqp_password=options_string_declare();
	broker_options->amqp_reconnect=false;
	broker_options->amqp_allow_sasl=false;
	broker_options->amqp_allow_insecure=false;

	broker_options->broker_url=options_string_declare();
	broker_options->broker_topic=options_string_declare();

	broker_options->amqp_idle_timeout=-1; // Disabled by default
	broker_options->amqp_reconnect_after_local_timeout_expired=false;
}

static void broker_options_free(broker_options_t *broker_options) {
	if(broker_options!=NULL) {
		options_string_free(broker_options->broker_url);
		options_string_free(broker_options->broker_topic);
		options_string_free(broker_options->amqp_username);
		options_string_free(broker_options->amqp_password);
	}
}

void options_initialize(struct options *options) {
	options->init_code=INIT_CODE;

	// Initialize here your options
	options->min_lat=INVALID_LONLAT;
	options->min_lon=INVALID_LONLAT;
	options->max_lat=INVALID_LONLAT;
	options->max_lon=INVALID_LONLAT;

	options->ext_lat_factor=0;
	options->ext_lon_factor=0;

	options->cross_border_trigger=false;

	broker_options_inizialize(&options->amqp_broker_one);

	// Additional AMQP clients options (num_amqp_x_enabled should always start from '0' here, i.e., no additional AMQP clients are used by default)
	// ----------------------------------
	options->amqp_broker_x=NULL;
	// for(int i=0;i<MAX_AMQP_CLIENTS-1;i++) {
	// 	// options->amqp_broker_x_enabled[i]=false;
	// 	broker_options_inizialize(&options->amqp_broker_x[i]);
	// }
	options->num_amqp_x_enabled=0;
	// ----------------------------------

	// options->broker_url=options_string_declare();
	// options->broker_topic=options_string_declare();

	options->ms_rest_addr=options_string_declare();
	options->ms_rest_port=DEFAULT_MANEUVERING_SERVICE_REST_SRV_PORT;
	options->left_indicator_trg_enable=false; // Only the right turn indicator is considered, by default
	options->ms_rest_periodicity=DEFAULT_MANEUVERING_SERVICE_REST_PERIODICITY;
	options->gn_timestamp_property=options_string_declare();
	options->ext_lights_hijack_enable=false; // Only the appropiate ext. lights field in the lowFreqContainer is used by default
	options->interop_hijack_enable=false; //Every vehicle is able to trigger the data to be sent via REST, with the correct stationType

	options->vehviz_nodejs_addr=options_string_declare();
	options->vehviz_nodejs_port=DEFAULT_VEHVIZ_NODEJS_UDP_PORT;

	options->vehviz_web_interface_port=DEFAULT_VEHVIZ_WEB_PORT;

	options->logfile_name=options_string_declare();

	options->context_radius=DEFAULT_CONTEXT_RADIUS_METERS;
	options->vehviz_update_interval_sec=DEFAULT_VEHVIZ_UPDATE_INTERVAL_SECONDS;

	options->indicatorTrgMan_enabled=true;

	options->ageCheck_enabled=true;
	options->quadkFilter_enabled=true;

	options->od_json_interface_enabled=false;
	options->od_json_interface_port=DEFAULT_OD_JSON_OVER_TCP_INTERFACE_PORT;
}

unsigned int parse_options(int argc, char **argv, struct options *options) {
	int char_option;
	bool version_flg=false;
	char *sPtr; // String pointer for strtod() calls

	if(options->init_code!=INIT_CODE) {
		fprintf(stderr,"parse_options: you are trying to parse the options without initialiting\n"
			"struct options, this is not allowed.\n");
		return 1;
	}

	while ((char_option=getopt_long(argc, argv, VALID_OPTS, long_opts, NULL)) != EOF) {
		switch(char_option) {
			case 0:
				fprintf(stderr,"Error. An unexpected error occurred when parsing the options.\n"
					"Please report to the developers that getopt_long() returned 0. Thank you.\n");
				exit(EXIT_FAILURE);
				break;

			case 'A':
				if(sscanf(optarg,"%lf:%lf-%lf:%lf",
					&options->min_lat,
					&options->min_lon,
					&options->max_lat,
					&options->max_lon)<4) {

					fprintf(stderr,"Error parsing the area string after the --area/-A option.");
					print_short_info_err(options,argv[0]);
				}
				break;

			case 'E':
				errno=0; // Setting errno to 0 as suggested in the strtod() man page
				options->ext_lat_factor=strtod(optarg,&sPtr);

				if(sPtr==optarg) {
					fprintf(stderr,"Cannot find any digit in the specified value (-E/" LONGOPT_E ").\n");
					print_short_info_err(options,argv[0]);
				} else if(errno || options->ext_lat_factor<0) {
					fprintf(stderr,"Error in parsing the latitude factor for the extended area (remember that it must be positive).\n");
					print_short_info_err(options,argv[0]);
				}
				break;

			case 'F':
				errno=0; // Setting errno to 0 as suggested in the strtod() man page
				options->ext_lon_factor=strtod(optarg,&sPtr);

				if(sPtr==optarg) {
					fprintf(stderr,"Cannot find any digit in the specified value (-F/" LONGOPT_F ").\n");
					print_short_info_err(options,argv[0]);
				} else if(errno || options->ext_lon_factor<0) {
					fprintf(stderr,"Error in parsing the longitude factor for the extended area (remember that it must be positive).\n");
					print_short_info_err(options,argv[0]);
				}
				break;

			case 'c':
				options->cross_border_trigger=true;
				break;

			case 'U':
				if(!options_string_push(&(options->amqp_broker_one.broker_url),optarg)) {
					fprintf(stderr,"Error in parsing the AMQP broker url/address.\n");
					print_short_info_err(options,argv[0]);
				}
				break;

			case 'Q':
				if(!options_string_push(&(options->amqp_broker_one.broker_topic),optarg)) {
					fprintf(stderr,"Error in parsing the AMQP broker queue/topic name.\n");
					print_short_info_err(options,argv[0]);
				}
				break;

			case 'u':
				if(!options_string_push(&(options->amqp_broker_one.amqp_username),optarg)) {
					fprintf(stderr,"Error in parsing the AMQP broker username.\n");
					print_short_info_err(options,argv[0]);
				}
				break;

			case 'p':
				if(!options_string_push(&(options->amqp_broker_one.amqp_password),optarg)) {
					fprintf(stderr,"Error in parsing the AMQP broker password.\n");
					print_short_info_err(options,argv[0]);
				}
				break;

			case 'R':
				options->amqp_broker_one.amqp_reconnect=true;
				break;

			case 'S':
				options->amqp_broker_one.amqp_allow_sasl=true;
				break;

			case 'I':
				options->amqp_broker_one.amqp_allow_insecure=true;
				break;

			case 'O':
				options->od_json_interface_enabled=true;
				break;

			case 'o':
				errno=0; // Setting errno to 0 as suggested in the strtol() man page
				options->od_json_interface_port=strtol(optarg,&sPtr,10);

				if(sPtr==optarg) {
					fprintf(stderr,"Cannot find any digit in the specified value (-o/" LONGOPT_o ").\n");
					print_short_info_err(options,argv[0]);
				} else if(errno || options->od_json_interface_port<1 || options->od_json_interface_port>65535) {
					// Only port numbers from 1 to 65535 are valid and can be accepted
					fprintf(stderr,"Error in parsing the port number for the on-demand REST interface server.\n");
					print_short_info_err(options,argv[0]);
				}
				break;

			case 'r':
				// fprintf(stdout,"[INFO] Specified a custom address for the Maneuvering Service REST server: %s\n",optarg);
				if(!options_string_push(&(options->ms_rest_addr),optarg)) {
					fprintf(stderr,"Error in parsing the Maneuvering Service REST server address. Did you specify it twice?\n");
					print_short_info_err(options,argv[0]);
				}
				break;

			case 's':
				errno=0; // Setting errno to 0 as suggested in the strtol() man page
				options->ms_rest_port=strtol(optarg,&sPtr,10);

				if(sPtr==optarg) {
					fprintf(stderr,"Cannot find any digit in the specified value (-s/" LONGOPT_s ").\n");
					print_short_info_err(options,argv[0]);
				} else if(errno || options->ms_rest_port<1 || options->ms_rest_port>65535) {
					// Only port numbers from 1 to 65535 are valid and can be accepted
					fprintf(stderr,"Error in parsing the Maneuvering Service REST server port.\n");
					print_short_info_err(options,argv[0]);
				}
				break;

			case 'Z':
				if(!options_string_push(&(options->vehviz_nodejs_addr),optarg)) {
					fprintf(stderr,"Error in parsing the IPv4 address for the UDP connection to the Vehicle Visualizer Node.js server.\n");
					print_short_info_err(options,argv[0]);
				}
				break;

			case 'z':
				errno=0; // Setting errno to 0 as suggested in the strtol() man page
				options->vehviz_nodejs_port=strtol(optarg,&sPtr,10);

				if(sPtr==optarg) {
					fprintf(stderr,"Cannot find any digit in the specified value (-z/" LONGOPT_z ").\n");
					print_short_info_err(options,argv[0]);
				} else if(errno || options->vehviz_nodejs_port<1 || options->vehviz_nodejs_port>65535) {
					// Only port numbers from 1 to 65535 are valid and can be accepted
					fprintf(stderr,"Error in parsing the port number for the UDP connection to the Vehicle Visualizer Node.js server.\n");
					print_short_info_err(options,argv[0]);
				}
				break;

			case 'w':
				errno=0; // Setting errno to 0 as suggested in the strtol() man page
				options->vehviz_web_interface_port=strtol(optarg,&sPtr,10);

				if(sPtr==optarg) {
					fprintf(stderr,"Cannot find any digit in the specified value (-w/" LONGOPT_w ").\n");
					print_short_info_err(options,argv[0]);
				} else if(errno || options->vehviz_web_interface_port<1 || options->vehviz_web_interface_port>65535) {
					// Only port numbers from 1 to 65535 are valid and can be accepted
					fprintf(stderr,"Error in parsing the port number for the web interface of the Vehicle Visualizer.\n");
					print_short_info_err(options,argv[0]);
				}
				break;

			case 'L':
				if(!options_string_push(&(options->logfile_name),optarg)) {
					fprintf(stderr,"Error in parsing the log file name: %s.\n",optarg);
					print_short_info_err(options,argv[0]);
				}
				break;

			case 'C':
				errno=0; // Setting errno to 0 as suggested in the strtod() man page
				options->context_radius=strtod(optarg,&sPtr);

				if(sPtr==optarg) {
					fprintf(stderr,"Cannot find any digit in the specified value (-C/" LONGOPT_C ").\n");
					print_short_info_err(options,argv[0]);
				} else if(errno || options->context_radius<MINIMUM_CONTEXT_RADIUS_METERS) {
					fprintf(stderr,"Error in parsing the context radius (it cannot be lower than 10 meters).\n");
					print_short_info_err(options,argv[0]);
				}
				break;

			case 'g':
				if(options->ageCheck_enabled==false) {
					fprintf(stderr,"Error. The age check (-g/--"LONGOPT_g") has already been disabled once.\n");
					print_short_info_err(options,argv[0]);
				}

				options->ageCheck_enabled=false;
				break;

			case LONGOPT_vehviz_update_interval_sec_val:
				errno=0; // Setting errno to 0 as suggested in the strtod() man page
				options->vehviz_update_interval_sec=strtod(optarg,&sPtr);

				if(sPtr==optarg) {
					fprintf(stderr,"Cannot find any digit in the specified value (--" LONGOPT_vehviz_update_interval_sec ").\n");
					print_short_info_err(options,argv[0]);
				} else if(errno || options->vehviz_update_interval_sec<0.05 || options->vehviz_update_interval_sec>1.0) {
					fprintf(stderr,"Error in parsing the update rate for the web-based GUI. Remember that it must be within [0.05,1] seconds.\n");
					print_short_info_err(options,argv[0]);
				}
				break;

			case LONGOPT_amqp_main_idle_timeout_val:
				errno=0; // Setting errno to 0 as suggested in the strtod() man page
				options->amqp_broker_one.amqp_idle_timeout=strtol(optarg,&sPtr,10);

				if(sPtr==optarg) {
					fprintf(stderr,"Cannot find any digit in the specified value (--" LONGOPT_vehviz_update_interval_sec ").\n");
					print_short_info_err(options,argv[0]);
				} else if(errno) {
					fprintf(stderr,"Error in parsing the main AMQP client idle timeout value. Probably a wrong format has been specified.\n");
					print_short_info_err(options,argv[0]);
				}

				break;

			case LONGOPT_indicator_trgman_disable_val:
				options->indicatorTrgMan_enabled=false;
				break;

			case LONGOPT_disable_quadkey_filter_val:
				options->quadkFilter_enabled=false;
				break;

			case LONGOPT_enable_left_indicator_trigger_val:
				options->left_indicator_trg_enable=true;
				break;

			case LONGOPT_amqp_main_reconn_local_timeout_exp_val:
				options->amqp_broker_one.amqp_reconnect_after_local_timeout_expired=true;
				break;

			case LONGOPT_ms_rest_periodicity_val:
				errno=0; // Setting errno to 0 as suggested in the strtod() man page
				options->ms_rest_periodicity=strtod(optarg,&sPtr);

				if(sPtr==optarg) {
					fprintf(stderr,"Cannot find any digit in the specified value (--" LONGOPT_ms_rest_periodicity ").\n");
					print_short_info_err(options,argv[0]);
				} else if(errno || options->ms_rest_periodicity<0.001 || options->ms_rest_periodicity>10.0) {
					fprintf(stderr,"Error in parsing the periodicity for the REST data transmission. Remember that it must be within [0.001,10.0] seconds.\n");
					print_short_info_err(options,argv[0]);
				}
				break;

			case LONGOPT_gn_timestamp_property_val:
				if(!options_string_push(&(options->gn_timestamp_property),optarg)) {
					fprintf(stderr,"Error in parsing the gn timestamp property name: %s.\n",optarg);
					print_short_info_err(options,argv[0]);
				}
				break;

			case LONGOPT_enable_ext_lights_hijack_val:
				options->ext_lights_hijack_enable=true;
				break;

			case LONGOPT_enable_interop_hijack_val:
				options->interop_hijack_enable=true;
				break;

			// Additional AMQP clients options
			// ----------------------------------
			case LONGOPT_amqp_enable_additionals_val:
				errno=0; // Setting errno to 0 as suggested in the strtol() man page
				options->num_amqp_x_enabled=strtol(optarg,&sPtr,10);

				if(sPtr==optarg) {
					fprintf(stderr,"Cannot find any digit in the specified value (--" LONGOPT_amqp_enable_additionals ").\n");
					options->num_amqp_x_enabled=0;
					print_short_info_err(options,argv[0]);
				} else if(errno || options->num_amqp_x_enabled<1 || options->num_amqp_x_enabled>MAX_ADDITIONAL_AMQP_CLIENTS-1) {
					fprintf(stderr,"Error in parsing the number of additional AMQP clients. Remember that this number must be within [1,9].\n");
					options->num_amqp_x_enabled=0;
					print_short_info_err(options,argv[0]);
				}

				// Allocate the memory for the additional AMQP clients options (as we know, now, how many clients have been requested by the user)
				options->amqp_broker_x=(broker_options_t *)malloc(options->num_amqp_x_enabled*sizeof(broker_options_t));
				if(options->amqp_broker_x==NULL) {
					fprintf(stderr,"Memory allocation error. Cannot allocate memory to store the additional AMQP clients options. Please free some memory and try again.\n");
					options->num_amqp_x_enabled=0;
					print_short_info_err(options,argv[0]);
				}

				for(int i=0;i<options->num_amqp_x_enabled;i++) {
					broker_options_inizialize(&options->amqp_broker_x[i]);
				}

				break;

			case LONGOPT_amqp_additionals_url_val:
				if(options->num_amqp_x_enabled<=0) {
					fprintf(stderr,"Error: attempting to configure additional AMQP clients before enabling them.\nYou must specify --" LONGOPT_amqp_enable_additionals " before attempting to configure any other additional AMQP client option.\n");
					print_short_info_err(options,argv[0]);
				}

				if(fill_AMQPClient_options_array_broker_url(optarg,options->num_amqp_x_enabled,options->amqp_broker_x)!=true) {
					fprintf(stderr,"Error in parsing the additional AMQP broker(s) URL. Please check if enough arguments are specified in the comma-separated list.\n");
					print_short_info_err(options,argv[0]);
				}
				break;

			case LONGOPT_amqp_additionals_queue_val:
				if(options->num_amqp_x_enabled<=0) {
					fprintf(stderr,"Error: attempting to configure additional AMQP clients before enabling them.\nYou must specify --" LONGOPT_amqp_enable_additionals " before attempting to configure any other additional AMQP client option.\n");
					print_short_info_err(options,argv[0]);
				}

				if(fill_AMQPClient_options_array_broker_topic(optarg,options->num_amqp_x_enabled,options->amqp_broker_x)!=true) {
					fprintf(stderr,"Error in parsing the additional AMQP broker(s) queue/topic. Please check if enough arguments are specified in the comma-separated list.\n");
					print_short_info_err(options,argv[0]);
				}
				break;

			case LONGOPT_amqp_additionals_username_val:
				if(options->num_amqp_x_enabled<=0) {
					fprintf(stderr,"Error: attempting to configure additional AMQP clients before enabling them.\nYou must specify --" LONGOPT_amqp_enable_additionals " before attempting to configure any other additional AMQP client option.\n");
					print_short_info_err(options,argv[0]);
				}

				if(fill_AMQPClient_options_array_amqp_username(optarg,options->num_amqp_x_enabled,options->amqp_broker_x)!=true) {
					fprintf(stderr,"Error in parsing the additional AMQP broker(s) username. Please check if enough arguments are specified in the comma-separated list.\n");
					print_short_info_err(options,argv[0]);
				}
				break;

			case LONGOPT_amqp_additionals_password_val:
				if(options->num_amqp_x_enabled<=0) {
					fprintf(stderr,"Error: attempting to configure additional AMQP clients before enabling them.\nYou must specify --" LONGOPT_amqp_enable_additionals " before attempting to configure any other additional AMQP client option.\n");
					print_short_info_err(options,argv[0]);
				}

				if(fill_AMQPClient_options_array_amqp_password(optarg,options->num_amqp_x_enabled,options->amqp_broker_x)!=true) {
					fprintf(stderr,"Error in parsing the additional AMQP broker(s) password. Please check if enough arguments are specified in the comma-separated list.\n");
					print_short_info_err(options,argv[0]);
				}
				break;

			case LONGOPT_amqp_additionals_reconnect_val:
				if(options->num_amqp_x_enabled<=0) {
					fprintf(stderr,"Error: attempting to configure additional AMQP clients before enabling them.\nYou must specify --" LONGOPT_amqp_enable_additionals " before attempting to configure any other additional AMQP client option.\n");
					print_short_info_err(options,argv[0]);
				}

				if(fill_AMQPClient_options_array_amqp_reconnect(optarg,options->num_amqp_x_enabled,options->amqp_broker_x)!=true) {
					fprintf(stderr,"Error in parsing the additional AMQP broker(s) reconnect options. Please check if enough arguments are specified in the comma-separated list.\n");
					print_short_info_err(options,argv[0]);
				}
				break;

			case LONGOPT_amqp_additionals_allow_sasl_val:
				if(options->num_amqp_x_enabled<=0) {
					fprintf(stderr,"Error: attempting to configure additional AMQP clients before enabling them.\nYou must specify --" LONGOPT_amqp_enable_additionals " before attempting to configure any other additional AMQP client option.\n");
					print_short_info_err(options,argv[0]);
				}

				if(fill_AMQPClient_options_array_amqp_allow_sasl(optarg,options->num_amqp_x_enabled,options->amqp_broker_x)!=true) {
					fprintf(stderr,"Error in parsing the additional AMQP broker(s) allow SASL options. Please check if enough arguments are specified in the comma-separated list.\n");
					print_short_info_err(options,argv[0]);
				}

				break;

			case LONGOPT_amqp_additionals_allow_plain_val:
				if(options->num_amqp_x_enabled<=0) {
					fprintf(stderr,"Error: attempting to configure additional AMQP clients before enabling them.\nYou must specify --" LONGOPT_amqp_enable_additionals " before attempting to configure any other additional AMQP client option.\n");
					print_short_info_err(options,argv[0]);
				}

				if(fill_AMQPClient_options_array_amqp_allow_insecure(optarg,options->num_amqp_x_enabled,options->amqp_broker_x)!=true) {
					fprintf(stderr,"Error in parsing the additional AMQP broker(s) allow PLAIN options. Please check if enough arguments are specified in the comma-separated list.\n");
					print_short_info_err(options,argv[0]);
				}

				break;

			case LONGOPT_amqp_additionals_idle_timeout_val:
				if(options->num_amqp_x_enabled<=0) {
					fprintf(stderr,"Error: attempting to configure additional AMQP clients before enabling them.\nYou must specify --" LONGOPT_amqp_enable_additionals " before attempting to configure any other additional AMQP client option.\n");
					print_short_info_err(options,argv[0]);
				}

				if(fill_AMQPClient_options_array_amqp_idle_timeout(optarg,options->num_amqp_x_enabled,options->amqp_broker_x)!=true) {
					fprintf(stderr,"Error in parsing the additional AMQP broker(s) idle timeout options. Please check if enough arguments are specified in the comma-separated list.\n");
					print_short_info_err(options,argv[0]);
				}
				break;

			case LONGOPT_amqp_additionals_reconn_local_timeout_exp_val:
				if(options->num_amqp_x_enabled<=0) {
					fprintf(stderr,"Error: attempting to configure additional AMQP clients before enabling them.\nYou must specify --" LONGOPT_amqp_enable_additionals " before attempting to configure any other additional AMQP client option.\n");
					print_short_info_err(options,argv[0]);
				}

				if(fill_AMQPClient_options_array_amqp_reconnect_after_local_timeout_expired(optarg,options->num_amqp_x_enabled,options->amqp_broker_x)!=true) {
					fprintf(stderr,"Error in parsing the additional AMQP broker(s) reconnect after local idle timeout options. Please check if enough arguments are specified in the comma-separated list.\n");
					print_short_info_err(options,argv[0]);
				}
				break;
			// ----------------------------------

			case 'v':
				version_flg = true;
				break;

			case 'h':
				print_long_info(argv[0]);
				break;

			default:
				print_short_info_err(options,argv[0]);
				return 1;

		}

	}

	if(version_flg==true) {
		fprintf(stdout,"Version: %s\n",VERSION_STR);
		exit(EXIT_SUCCESS);
	}

	// Additional AMQP clients options check
	// ----------------------------------
	for(int i=0;i<options->num_amqp_x_enabled;i++) {
		if(options_string_len(options->amqp_broker_x[i].broker_url)<=0) {
			fprintf(stderr,"Error! No broker url/address specified for additional AMQP client %d.\nWhen using additional AMQP clients it is mandatory to specify a broker URL and queue/topic.\n",i+2);
			exit(EXIT_FAILURE);
		}

		if(options_string_len(options->amqp_broker_x[i].broker_topic)<=0) {
			fprintf(stderr,"Error! No broker queue/topic specified for additional AMQP client %d.\nWhen using additional AMQP clients it is mandatory to specify a broker URL and queue/topic.\n",i+2);
			exit(EXIT_FAILURE);
		}

		// Print some additional information
		if(options_string_len(options->amqp_broker_x[i].amqp_username)>1) {
			fprintf(stdout,"AMQP client %d has a username set.\n",i+2);
		} else {
			fprintf(stdout,"No username set for AMQP client %d.\n",i+2);
		}

		if(options_string_len(options->amqp_broker_x[i].amqp_password)>1) {
			fprintf(stdout,"AMQP client %d has a password set.\n",i+2);
		} else {
			fprintf(stdout,"No password set for AMQP client %d.\n",i+2);
		}
	}
	// ----------------------------------

	// Set the default values for the "options_string" options, if they have not been set before
	// An unset "options_string" has a length which is <= 0
	if(options_string_len(options->amqp_broker_one.broker_url)<=0) {
		if(!options_string_push(&(options->amqp_broker_one.broker_url),DEFAULT_BROKER_URL)) {
			fprintf(stderr,"Error! Cannot set the default broker url/address.\nPlease report this bug to the developers.\n");
			exit(EXIT_FAILURE);
		}
	}

	if(options_string_len(options->amqp_broker_one.broker_topic)<=0) {
		if(!options_string_push(&(options->amqp_broker_one.broker_topic),DEFAULT_BROKER_QUEUE)) {
			fprintf(stderr,"Error! Cannot set the default broker queue/topic.\nPlease report this bug to the developers.\n");
			exit(EXIT_FAILURE);
		}
	}

	if(options_string_len(options->ms_rest_addr)<=0) {
		if(!options_string_push(&(options->ms_rest_addr),DEFAULT_MANEUVERING_SERVICE_REST_SRV_ADDR)) {
			fprintf(stderr,"Error! Cannot set the default Maneuvering Service REST server address.\nPlease report this bug to the developers.\n");
			exit(EXIT_FAILURE);
		}
	}

	if(options_string_len(options->vehviz_nodejs_addr)<=0) {
		if(!options_string_push(&(options->vehviz_nodejs_addr),DEFAULT_VEHVIZ_NODEJS_UDP_ADDR)) {
			fprintf(stderr,"Error! Cannot set the default IPv4 address for the UDP connection to the Vehicle Visualizer Node.js server.\nPlease report this bug to the developers.\n");
			exit(EXIT_FAILURE);
		}
	}

	if(options_string_len(options->gn_timestamp_property)<=0) {
		if(!options_string_push(&(options->gn_timestamp_property),DEFAULT_GN_TIMESTAMP_PROPERTY)) {
			fprintf(stderr,"Error! Cannot set the default gn timestamp property name.\nPlease report this bug to the developers.\n");
			exit(EXIT_FAILURE);
		}
	}

	// Check here the consistency of the options (e.g. if an option is not compatible with others, ...)
	// Check the validity of the "min_lat" value (as latitude value it should be inside [-90,90])
	// The case "options->min_lat < -90" also checks for an unset value of latitude (as INVALID_LONLAT is
	// equal to -DBL_MAX which is surely < -90)
	if(options->min_lat < -90 || options->min_lat > 90) {
		fprintf(stderr,"Error: invalid or unset minimum latitude value. Valid values range from -90 to 90.\n");
		print_short_info_err(options,argv[0]);
	}

	// Do the same for the maximum latitude value of the specified area
	if(options->max_lat < -90 || options->max_lat > 90) {
		fprintf(stderr,"Error: invalid or unset maximum latitude value. Valid values range from -90 to 90.\n");
		print_short_info_err(options,argv[0]);
	}

	// Check the validity of the "min_lon" value (as longitude value it should be inside [-180,180])
	// The case "options->min_lat < -180" also checks for an unset value of latitude (as INVALID_LONLAT is
	// equal to -DBL_MAX which is surely < -180)
	if(options->min_lon < -180 || options->min_lon > 180) {
		fprintf(stderr,"Error: invalid or unset minimum longitude value. Valid values range from -180 to 180.\n");
		print_short_info_err(options,argv[0]);
	}

	// Do the same for the maximum longitude value of the specified area
	if(options->max_lon < -180 || options->max_lon > 180) {
		fprintf(stderr,"Error: invalid or unset maximum longitude value. Valid values range from -180 to 180.\n");
		print_short_info_err(options,argv[0]);
	}

	return 0;
}

void options_free(struct options *options) {
	// If you allocated any memory with malloc() (remember that this module is written in C, not C++)
	// or if you used "options_string_push", you need to free the allocated memory here, either with
	// free() (for "manual" malloc()) or with options_string_free() if you are using "options_string"
	// data structures
	if(options!=NULL) {
		broker_options_free(&options->amqp_broker_one);

		// Additional AMQP clients options
		// ----------------------------------
		for(int i=0;i<options->num_amqp_x_enabled;i++) {
			broker_options_free(&options->amqp_broker_x[i]);
		}

		if(options->amqp_broker_x!=NULL) {
			free(options->amqp_broker_x);
		}
		// ----------------------------------

		options_string_free(options->ms_rest_addr);
		options_string_free(options->vehviz_nodejs_addr);
	}
}
