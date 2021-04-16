#include <iostream>
#include <unistd.h>

#include "LDMmap.h"
#include "vehicle-visualizer.h"

int main(int argc, char **argv) {
	// Create a new DB object
	ldmmap::LDMMap db;
	ldmmap::LDMMap::LDMMap_error_t db_retval;

	// Create a new vehicle visualizer object
	vehicleVisualizer vehicleVisObj;

	vehicleVisObj.startServer();
	vehicleVisObj.connectToServer ();

	// Sample vehicle
	ldmmap::vehicleData_t veh1 = {188321312, 45.562149, 8.055311, 400, -170, 1216424682444333};
	db.insert(veh1);

	// Draw the sample vehicle on the map (simulating 5 updates)
	vehicleVisObj.sendMapDraw(45.562149, 8.055311);
	vehicleVisObj.sendObjectUpdate("veh1",45.562149, 8.055311);
	sleep(1);
	vehicleVisObj.sendObjectUpdate("veh1",45.562139, 8.055311);
	sleep(1);
	vehicleVisObj.sendObjectUpdate("veh1",45.562129, 8.055311);
	sleep(1);
	vehicleVisObj.sendObjectUpdate("veh1",45.562119, 8.055311);
	sleep(1);
	vehicleVisObj.sendObjectUpdate("veh1",45.562109, 8.055311);

	std::cout << "Press a button to terminate this sample main()..." << std::endl;
	std::getchar();

	db.clear();

	return 0;
}