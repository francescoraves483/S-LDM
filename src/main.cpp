#include <iostream>
#include <unistd.h>

#include "LDMmap.h"
#include "vehicle-visualizer.h"
#include "QuadKeyTS.h"
#include "AMQPclient.h"

extern "C" {
	#include "options.h"
	#include "CAM.h"
}

#include "etsiDecoderFrontend.h"

int main(int argc, char **argv) {
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


    unsigned char *ptr = &cam[0];

    etsiDecoder::decoderFrontend decodeFrontend;
    etsiDecoder::etsiDecodedData_t decodedData;

    if(decodeFrontend.decodeEtsi(ptr, 85, decodedData)!=ETSI_DECODER_OK) {
    	std::cerr << "Error! Cannot decode ETSI packet!" << std::endl;
    }

    if(decodedData.type == etsiDecoder::ETSI_DECODED_CAM) {
    	CAM_t *decoded_cam;

    	decoded_cam = (CAM_t *) decodedData.decoded_msg;

    	printf("GNTimestamp: %u\n",decodedData.gnTimestamp);

    	printf("Lat: %.7lf, Lon: %.7lf\n",
    		(double)decoded_cam->cam.camParameters.basicContainer.referencePosition.latitude/10000000.0,
    		(double) decoded_cam->cam.camParameters.basicContainer.referencePosition.longitude/10000000.0);
    }

	/* ------------------------------------------------------------------------------------------------------------------------------------ */
	/* ------------------------------------------------------------------------------------------------------------------------------------ */
	/* ------------------------------------------------------------------------------------------------------------------------------------ */
	/* ------------------------------------------------------------------------------------------------------------------------------------ */
	/* ------------------------------------------------------------------------------------------------------------------------------------ */

	// Create a new DB object
	ldmmap::LDMMap *db_ptr = new ldmmap::LDMMap();

	// Before starting the AMQP client event loop, we should implement here the creation of a parallel thread, reading periodically 
	// (e.g. every 1/2 s) the database through the pointer "db_ptr" and "cleaning" the entries which are too old

	// We should also start here a second parallel thread, reading periodically the database (e.g. every 500 ms) and sending the vehicle data to
	// the vehicleVisualizer

	// All these additional thread should start with the "detached" attribute (?)

	// Start the AMQP client event loop (for the time being, on loopback, but some options will be added in the future)
    try {
        std::string conn_url = argc > 1 ? argv[1] : "127.0.0.1:5672";
        std::string addr = argc > 2 ? argv[2] : "examples";


        AMQPClient recvClient("127.0.0.1:5672", "examples", sldm_opts.min_lat, sldm_opts.max_lat, sldm_opts.min_lon, sldm_opts.max_lon, 16, &sldm_opts, db_ptr);
        proton::container(recvClient).run();

        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

	db_ptr->clear();

	return 0;
}