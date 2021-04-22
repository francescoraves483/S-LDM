#include <iostream>
#include <unistd.h>

#include "LDMmap.h"
#include "vehicle-visualizer.h"
#include "QuadKeyTS.h"

extern "C" {
	#include "options.h"
}

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

	// Create a new DB object
	ldmmap::LDMMap db;
	ldmmap::LDMMap::LDMMap_error_t db_retval;

	// Create a new vehicle visualizer object
	vehicleVisualizer vehicleVisObj;

	// Quadkey calculation object
	QuadKeys::QuadKeyTS quadcalc;

	vehicleVisObj.startServer();
	vehicleVisObj.connectToServer ();

	// Sample vehicle
	ldmmap::vehicleData_t veh1 = {188321312, 45.562149, 8.055311, 400, -170, 1216424682444333};
	db.insert(veh1);

	// Draw the sample vehicle on the map (simulating 5 updates)
	vehicleVisObj.sendMapDraw(45.562149, 8.055311);
	vehicleVisObj.sendObjectUpdate("veh1",45.562149, 8.055311);
	// sleep(1);
	// vehicleVisObj.sendObjectUpdate("veh1",45.562139, 8.055311);
	// sleep(1);
	// vehicleVisObj.sendObjectUpdate("veh1",45.562129, 8.055311);
	// sleep(1);
	// vehicleVisObj.sendObjectUpdate("veh1",45.562119, 8.055311);
	// sleep(1);
	// vehicleVisObj.sendObjectUpdate("veh1",45.562109, 8.055311);

	// Sample quadkey calculation
	std::cout << "The quadkey for (45.562109,8.055311) with detail level = 15 is: " << quadcalc.LatLonToQuadKey(45.562109,8.055311,15) << std::endl;

	std::cout << "Press a button to terminate this sample main()..." << std::endl;
	std::getchar();

	db.clear();

	return 0;
}