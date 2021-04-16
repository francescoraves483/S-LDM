#ifndef QUADKEYTILESYSTEM_H
#define QUADKEYTILESYSTEM_H

#include <string>
#include <iostream>
#include <sstream>

namespace QuadKeys
{
    class QuadKeyTS
    {
        private:
        	double Clip(double n, double minValue, double maxValue);

        public:
        	unsigned int MapSize(int levelOfDetail);
        	std::string LatLonToQuadKey(double latitude, double longitude, int levelOfDetail);
        	std::string LatLonToQuadKeyRange(double min_latitude, double max_latitude, double min_longitude, double max_longitude, int levelOfDetail);
    };
}

#endif // QUADKEYTILESYSTEM_H