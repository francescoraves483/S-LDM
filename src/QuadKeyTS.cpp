#include <string>
#include <iostream>
#include <sstream>
#include <cmath>

#include "QuadKeyTS.h"

namespace QuadKeys
{
	double 
	QuadKeyTS::Clip(double n, double minValue, double maxValue) {
		return std::min(std::max(n, minValue), maxValue);
	}

	unsigned int 
	QuadKeyTS::MapSize(int levelOfDetail) {
		return (unsigned int) 256 << levelOfDetail;
	}

	std::string 
	QuadKeyTS::LatLonToQuadKey(double latitude, double longitude, int levelOfDetail) {
		std::stringstream quadKey;

		double x = (longitude + 180) / 360;
		double sinLatitude = sin(latitude * M_PI / 180);
		double y = 0.5 - log((1 + sinLatitude) / (1 - sinLatitude)) / (4 * M_PI);

		unsigned int mapSize = MapSize(levelOfDetail);
		int pixelX = (int) Clip(x * mapSize + 0.5, 0, mapSize - 1);
		int pixelY = (int) Clip(y * mapSize + 0.5, 0, mapSize - 1);
		int tileX =  pixelX / 256;
		int tileY =  pixelY / 256;

		for (int i = levelOfDetail; i > 0; i--) {
			char digit = '0';
			int mask = 1 << (i - 1);
			if ((tileX & mask) != 0)
			{
				digit++;
			}
			if ((tileY & mask) != 0)
			{
				digit++;
				digit++;
			}
			quadKey << digit;
		}

		return quadKey.str();
	}

	std::string 
	QuadKeyTS::LatLonToQuadKeyRange(double min_latitude, double max_latitude, double min_longitude, double max_longitude, int levelOfDetail) {
		std::string quadKey_v1 = "";
		std::string quadKey_v2 = "";
		std::string quadKey_v3 = "";
		std::string quadKey_v4 = "";
		std::string quadKey_max = "";
		std::string quadKey_min = "";
		std::string quadKey = "";

		double x_v1 = (min_longitude + 180) / 360;
		double sinLatitude_v1 = sin(max_latitude * M_PI / 180);
		double y_v1 = 0.5 - log((1 + sinLatitude_v1) / (1 - sinLatitude_v1)) / (4 * M_PI);

		unsigned int mapSize_v1 = MapSize(levelOfDetail);
		int pixelX_v1 = (int) Clip(x_v1 * mapSize_v1 + 0.5, 0, mapSize_v1 - 1);
		int pixelY_v1 = (int) Clip(y_v1 * mapSize_v1 + 0.5, 0, mapSize_v1 - 1);
		int tileX_v1 =  pixelX_v1 / 256;
		int tileY_v1 =  pixelY_v1 / 256;


		for (int i = levelOfDetail; i > 0; i--) {
			char digit = '0';
			int mask = 1 << (i - 1);
			if ((tileX_v1 & mask) != 0) {
				digit++;
			}

			if ((tileY_v1 & mask) != 0) {
				digit++;
				digit++;
			}

			quadKey_v1 += digit;
		}

		// std::cout<< "First vertex of the squared area of interest: "<<quadKey_v1<<std::endl;

		double x_v2 = (max_longitude + 180) / 360;
		double sinLatitude_v2 = sin(max_latitude * M_PI / 180);
		double y_v2 = 0.5 - log((1 + sinLatitude_v2) / (1 - sinLatitude_v2)) / (4 * M_PI);

		unsigned int mapSize_v2 = MapSize(levelOfDetail);
		int pixelX_v2 = (int) Clip(x_v2 * mapSize_v2 + 0.5, 0, mapSize_v2 - 1);
		int pixelY_v2 = (int) Clip(y_v2 * mapSize_v2 + 0.5, 0, mapSize_v2 - 1);
		int tileX_v2 =  pixelX_v2 / 256;
		int tileY_v2 =  pixelY_v2 / 256;


		for (int i = levelOfDetail; i > 0; i--) {
			char digit = '0';
			int mask = 1 << (i - 1);
			if ((tileX_v2 & mask) != 0) {
				digit++;
			}

			if ((tileY_v2 & mask) != 0) {
				digit++;
				digit++;
			}

			quadKey_v2 += digit;
		}

		// std::cout<< "Second vertex of the squared area of interest: "<<quadKey_v2 <<std::endl;

		double x_v3 = (min_longitude + 180) / 360;
		double sinLatitude_v3 = sin(min_latitude * M_PI / 180);
		double y_v3 = 0.5 - log((1 + sinLatitude_v3) / (1 - sinLatitude_v3)) / (4 * M_PI);

		unsigned int mapSize_v3 = MapSize(levelOfDetail);
		int pixelX_v3 = (int) Clip(x_v3 * mapSize_v3 + 0.5, 0, mapSize_v3 - 1);
		int pixelY_v3 = (int) Clip(y_v3 * mapSize_v3 + 0.5, 0, mapSize_v3 - 1);
		int tileX_v3 =  pixelX_v3 / 256;
		int tileY_v3 =  pixelY_v3 / 256;


		for (int i = levelOfDetail; i > 0; i--) {
			char digit = '0';
			int mask = 1 << (i - 1);
			if ((tileX_v3 & mask) != 0)
			{
				digit++;
			}
			if ((tileY_v3 & mask) != 0)
			{
				digit++;
				digit++;
			}

			quadKey_v3 += digit;
		}

		// std::cout<<"Third vertex of the squared area of interest: " <<quadKey_v3 <<std::endl;

		double x_v4 = (max_longitude + 180) / 360;
		double sinLatitude_v4 = sin(min_latitude * M_PI / 180);
		double y_v4 = 0.5 - log((1 + sinLatitude_v4) / (1 - sinLatitude_v4)) / (4 * M_PI);

		unsigned int mapSize_v4 = MapSize(levelOfDetail);
		int pixelX_v4 = (int) Clip(x_v4 * mapSize_v4 + 0.5, 0, mapSize_v4 - 1);
		int pixelY_v4 = (int) Clip(y_v4 * mapSize_v4 + 0.5, 0, mapSize_v4 - 1);
		int tileX_v4 =  pixelX_v4 / 256;
		int tileY_v4 =  pixelY_v4 / 256;


		for (int i = levelOfDetail; i > 0; i--) {
			char digit = '0';
			int mask = 1 << (i - 1);
			if ((tileX_v4 & mask) != 0) {
				digit++;
			}

			if ((tileY_v4 & mask) != 0) {
				digit++;
				digit++;
			}

			quadKey_v4 += digit;
		}

		// std::cout<< "Fourth vertex of the squared area of interest: "<<quadKey_v4 <<std::endl;


		// Here we compare the generated quadkeys
		for(int i = 0; i < levelOfDetail; i++) {
			if(quadKey_v1[i] == quadKey_v2[i]) {
				quadKey_max += quadKey_v1[i];
			} else {
				break;
			}
		}

		for(int i = 0; i < levelOfDetail; i++) {
			if(quadKey_v3[i] == quadKey_v4[i]) {
				quadKey_min += quadKey_v3[i];
			} else {
				break;
			}
		}

		for(int i = 0; i < levelOfDetail && i < (int) quadKey_min.length(); i++) {
			if(quadKey_min[i] == quadKey_max[i]){
				quadKey += quadKey_min[i];
			} else{
				break;
			}
		}

		// std::cout<<"The quadkey of the area of interest is: " << quadKey << "%" <<std::endl;

		return quadKey;
	}
}