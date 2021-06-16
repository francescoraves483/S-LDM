#include "options.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

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
	{NULL, 0, NULL, 0}
};

// Option strings: defined here the description for each option to be then included inside print_long_info()
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
	"  -Z: advanced option: set the IPv4 address for the UDP connection to the Vehicle Visualizer\n" \
	"\t  Node.js server (excluding the port number).\n" \
	"\t  This is the address without port number. Default: ("DEFAULT_VEHVIZ_NODEJS_UDP_ADDR").\n"

#define OPT_z_description \
	LONGOPT_STR_CONSTRUCTOR(LONGOPT_z) \
	"  -z: advanced option: set the port number for the UDP connection to the Vehicle Visualizer\n" \
	"\t  Node.js server.\n" \
	"\t  Default: ("STRINGIFY(DEFAULT_VEHVIZ_NODEJS_UDP_PORT)").\n"

#define OPT_w_description \
	LONGOPT_STR_CONSTRUCTOR(LONGOPT_w) \
	"  -w: set the port at which the web interface of the Vehicle Visualizer will be available.\n" \
	"\t  Default: ("STRINGIFY(DEFAULT_VEHVIZ_WEB_PORT)").\n"


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

	options->broker_url=options_string_declare();
	options->broker_topic=options_string_declare();

	options->ms_rest_addr=options_string_declare();
	options->ms_rest_port=DEFAULT_MANEUVERING_SERVICE_REST_SRV_PORT;

	options->vehviz_nodejs_addr=options_string_declare();
	options->vehviz_nodejs_port=DEFAULT_VEHVIZ_NODEJS_UDP_PORT;

	options->vehviz_web_interface_port=DEFAULT_VEHVIZ_WEB_PORT;
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
				if(!options_string_push(&(options->broker_url),optarg)) {
					fprintf(stderr,"Error in parsing the AMQP broker url/address.\n");
					print_short_info_err(options,argv[0]);
				}
				break;

			case 'Q':
				if(!options_string_push(&(options->broker_topic),optarg)) {
					fprintf(stderr,"Error in parsing the AMQP broker queue/topic name.\n");
					print_short_info_err(options,argv[0]);
				}
				break;

			case 'r':
				if(!options_string_push(&(options->ms_rest_addr),optarg)) {
					fprintf(stderr,"Error in parsing the Maneuvering Service REST server address.\n");
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

	// Set the default values for the "options_string" options, if they have not been set before
	// An unset "options_string" has a length which is <= 0
	if(options_string_len(options->broker_url)<=0) {
		if(!options_string_push(&(options->broker_url),DEFAULT_BROKER_URL)) {
			fprintf(stderr,"Error! Cannot set the default broker url/address.\nPlease report this bug to the developers.\n");
			exit(EXIT_FAILURE);
		}
	}

	if(options_string_len(options->broker_topic)<=0) {
		if(!options_string_push(&(options->broker_topic),DEFAULT_BROKER_QUEUE)) {
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
		options_string_free(options->broker_url);
		options_string_free(options->broker_topic);
		options_string_free(options->ms_rest_addr);
		options_string_free(options->vehviz_nodejs_addr);
	}
}