#include <string>
#include <iostream>
#include <sstream>
#include <cmath>
#include <numeric>
#include "QuadKeyTS.h"
#include <fstream>

namespace QuadKeys
{
	QuadKeyTS::QuadKeyTS() {
		// Set the default level of detail and corresponding needed lat-lon variation
		m_levelOfDetail = 16;
		m_latlon_variation = 0.001;
	}

	double 
	QuadKeyTS::Clip(double n, double minValue, double maxValue) {
		return std::min(std::max(n, minValue), maxValue);
	}

	unsigned int 
	QuadKeyTS::MapSize(int levelOfDetail) {
		return (unsigned int) 256 << levelOfDetail;
	}

	bool 
	QuadKeyTS::finder(std::string vi, std::string vk, int l, std::array<int,4> &chars) {
		std::array<int,4> myints = {1,2,3,4};

		if(vi[l] == '0') {
			chars[0] = 1;
		}
		else if(vi[l] == '1') {
			chars[1] = 2;
		}
		else if(vi[l] == '2') {
			chars[2] = 3;
		}
		else if(vi[l] == '3') {
			chars[3] = 4;
		}

		if(vk[l] == '0') {
			chars[0] = 1;
		}
		else if(vk[l] == '1') {
			chars[1] = 2;
		}
		else if(vk[l] == '2') {
			chars[2] = 3;
		}
		else if(vk[l] == '3') {
			chars[3] = 4;
		}

		/*for(int i=0;i<4;i++){
			std::cout<<chars[i]<<" ";
		}*/

		if(chars == myints){
			chars = {};
			return 1;
		}
		else {
			return 0;
		}
	}

	// Here we distinguish the cases and set the private attributes on the base of desired levelOfDet
	void 
	QuadKeyTS::setLevelOfDetail(int levelOfDetail) {
		m_levelOfDetail = levelOfDetail;

		if(levelOfDetail < 14){
			m_levelOfDetail = 14;
		}
		if(levelOfDetail > 18){
			m_levelOfDetail = 18;
		}

		switch(m_levelOfDetail){

		case 18:
			m_latlon_variation = 0.001; 
			break;
		case 17:
			m_latlon_variation = 0.002;
			break;
		case 16:
			m_latlon_variation = 0.004;
			break;
		case 15:
			m_latlon_variation = 0.008;
			break;
		case 14:
			m_latlon_variation = 0.016;
			break;
		default: // We should never reach this point
			m_latlon_variation = 10000;
			break;
		}
	}

	std::string 
	QuadKeyTS::LatLonToQuadKey(double latitude, double longitude) {
		std::stringstream quadKey;
		// int levelOfDetail = ...; // The value of the desired zoom is obtained directly from the private attribute

		double x = (longitude + 180) / 360;
		double sinLatitude = sin(latitude * M_PI / 180);
		double y = 0.5 - log((1 + sinLatitude) / (1 - sinLatitude)) / (4 * M_PI);

		uint mapSize = MapSize(m_levelOfDetail);
		int pixelX = (int) Clip(x * mapSize + 0.5, 0, mapSize - 1);
		int pixelY = (int) Clip(y * mapSize + 0.5, 0, mapSize - 1);
		int tileX =  pixelX / 256;
		int tileY =  pixelY / 256;

		for (int i = m_levelOfDetail; i > 0; i--) {
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

		std::cout<<"[LatLonToQuadKey]: Sending: "<<quadKey.str()<<std::endl;
		return quadKey.str();
	}

	std::vector<std::string> 
	QuadKeyTS::LatLonToQuadKeyRange(double min_latitude, double max_latitude, double min_longitude, double max_longitude) {
		// int levelOfDetail = ...; // The value is now obtained directly from the private attribute
		//std::cout<<"\nCurrent variation: "<<m_latlon_variation<<"\nCurrent levelOfDetail: "<<m_levelOfDetail<<std::endl;
		std::string quadKey;
		std::vector<std::string> v = {};

		//starting scan the lan_lon using the private attribute m_latlon_variation
		for(double j = min_latitude; j <= max_latitude; j+=m_latlon_variation){
			for(double k = min_longitude; k <= max_longitude; k+=m_latlon_variation){

					quadKey = "";
					double x = (k + 180) / 360;
					double sinLatitude = sin(j * M_PI / 180);
					double y = 0.5 - log((1 + sinLatitude) / (1 - sinLatitude)) / (4 * M_PI);
					uint mapSize = MapSize(m_levelOfDetail);
					int pixelX = (int) Clip(x * mapSize + 0.5, 0, mapSize - 1);
					int pixelY = (int) Clip(y * mapSize + 0.5, 0, mapSize - 1);
					int tileX =  pixelX / 256;
					int tileY =  pixelY / 256;

					for (int i = m_levelOfDetail; i > 0; i--) {
					char digit = '0';
					int mask = 1 << (i - 1);
					if ((tileX & mask) != 0) {
						digit++;
					}
					if ((tileY & mask) != 0) {
						digit++;
						digit++;
					}
					quadKey += digit;
				}
				v.push_back(quadKey);
			}
		}

		//clear the vector to construct a desired one for testing

		/*v.clear();

		v.push_back("21000");
		v.push_back("21001");
		v.push_back("21002");
		v.push_back("21003");
		v.push_back("21010");
		v.push_back("21011");
		v.push_back("21012");
		v.push_back("21013");
		v.push_back("21020");
		v.push_back("21021");
		v.push_back("21022");
		v.push_back("21023");
		v.push_back("21030");
		v.push_back("21031");
		v.push_back("21032");
		v.push_back("22211");
		v.push_back("21033");*/

		for(size_t i = 0; i < v.size(); i++){
            for(size_t k = 0; k < v.size(); k++){
                if(i!=k && v.at(i) == v.at(k)){
                    auto it = v.begin();
                    v.erase(it + k );
                    i--;
                    break;
                }
            }
        }

        // Commented as it is intended only for debug purposes
        // firstvec.txt contains the initial vector of Quadkey
		// std::ofstream fout("firstvec.txt");
		//  for(int i = 0; i<v.size(); i++){
		//  	 fout << v.at(i) << "\n";
		//  }
		//  fout.close();
		
		return v;
	}

	void QuadKeyTS::unifyQuadkeys(std::vector<std::string> &quadKeys) {
		size_t k;
		bool loop = true;
		std::array<int,4> chars;
		bool complete;
		std::vector<std::string> vf = {};
		std::vector<std::string> vk = {};

		while(loop){
			//std::cout<<"loop\n";
			size_t i = 0;
			vf.clear();
			loop = false;
			//int l = v.at(i).size() - 2;
			//found = 0;

			while(i <= quadKeys.size()-1) {
				k = 0;
				//found = 0;
				int l = quadKeys.at(i).size() - 2;
				complete = false;
				chars = {};

				while(k <= (quadKeys.size() - 1)) {
					std::array<int,4> *charsp = &chars;

					if(i != k && quadKeys.at(i).compare(0,l+1,quadKeys.at(k),0,l+1) == 0){

						if(quadKeys.at(i).size() != quadKeys.at(k).size()){
							k++;
							continue;
						}

						vk.push_back(quadKeys.at(k).substr(0,l+1));

						if(finder(quadKeys.at(i), quadKeys.at(k), l+1, *charsp)){
							//std::cout<<"complete"<<std::endl;
							complete = true;
							chars = {};
						}
					}

					if(k == quadKeys.size() - 1 && !complete) {
						//std::cout<<"printing on final\n";
						vf.push_back(quadKeys.at(i).substr(0,l+2));
						vk.clear();
						break;
					}

					if(k == quadKeys.size() - 1 && complete) {
						size_t j = 0;
						while(j<=vk.size()-1){
							if(quadKeys.at(i).compare(0,l+1,vk.at(j),0,l+1) == 0){
								if(finder(quadKeys.at(i), vk.at(j), l+1, *charsp )){
									l--;
									chars = {};
								}
							j++;
							}
						}
						vf.push_back(quadKeys.at(i).substr(0,l+1));
						loop = true;
						vk.clear();
						break;
					}
					k++;
				}
				i++;
			}

			quadKeys = vf;
			//std::cout<<loop<<std::endl;
		}

		//erasing egual elements
		for(size_t i = 0; i < quadKeys.size(); i++){
			for(size_t k = 0; k < quadKeys.size(); k++){
				if(i!=k && quadKeys.at(i) == quadKeys.at(k)){
					auto it = quadKeys.begin();
					quadKeys.erase(it + k );
					i--;
					break;
				}
			}
		}

		//std::cout<<"Vector erased, checking dimension: "<< total + 21*(vf.size()-1) + 17 + 180 <<std::endl; //21 chars are added for every vector's string, while for the last one 17 chars are added insted. 180 is the offset btw the computed size and the one given by the error.

        // This is commented as these lines of code are just intended for debug purposes
        // midvec.txt contains the Quadkey vector content after the unify phase
        // std::ofstream fout("midvec.txt");
        // for(int i = 0; i < quadKeys.size(); i++){
        //     fout << quadKeys.at(i) << "\n";
        // }
        // fout.close();
	}

	void QuadKeyTS::checkdim(std::vector<std::string> &quadKeys) {
        std::vector<std::string> vf = quadKeys;
		const int max_length = 7000; // 131072; // 222; maximum length in byte to transfer infos to the broker
		unsigned int l = m_levelOfDetail; // 5;
		//int vec_len = vf.size();
        std::cout<<"[QUADKEYS] Checking dimensions: "<<std::accumulate(vf.begin(), vf.end(), -4, [](int sum, const std::string& elem) {return sum + elem.size() + 21;}) + 180<<std::endl;
		while(std::accumulate(vf.begin(), vf.end(), -4, [](int sum, const std::string& elem) {return sum + elem.size() + 21;}) + 180 >= max_length ){

            std::cout<<"[QUADKEYS] Dimensions exceeded, resizing:"<<std::endl;
            int flag = 0;

            if(std::accumulate(vf.begin(), vf.end(), -4, [](int sum, const std::string& elem) {return sum + elem.size() + 21;}) + 180 > (1.5*max_length)){
                for(size_t j = 0; j < vf.size(); j++){

                	if(vf.at(j).size() == l){
                		auto it = vf.begin();
                		vf.erase(it + j);
                	}
                }

                //l--;
            }
            else{

            	for (size_t j = 0; j < vf.size(); j++) {

                    if(vf.at(j).size() ==  l) {
                        //std::cout<<"found"<<vf.at(j).size() <<std::endl;
                        flag++;
                        vf.push_back(vf.at(j).substr(0,l-1));
                        auto it = vf.begin();
                        vf.erase(it + j);
                        break;
                    }
                    if(j == vf.size() - 1 && flag == 0){
                        l--;
                    }
                }
            }		    

		    std::cout<<"[QUADKEYS] New dimension: "<< std::accumulate(vf.begin(), vf.end(), -4, [](int sum, const std::string& elem) {return sum + elem.size() + 21;}) + 180 <<std::endl;

		    if(l<0) {
		    	break;
		    }
		}

		//QuadKeyTS::unifyQuadkeys(vf);
        for(size_t i = 0; i < vf.size(); i++){
			for(size_t k = 0; k < vf.size(); k++){
				if(i!=k && vf.at(i) == vf.at(k)){
				    auto it = vf.begin();
				    vf.erase(it + k );
				    i--;
				    break;
			    }
			}
		}
		
		quadKeys = vf;

		// This is commented as it is intended only for debug purposes
		// lastvec.txt contains the final Quadkey vector content after the size reduction phase (if it happens)
		//std::cout<<"\nFinished\n";
        // std::ofstream fout("lastvec.txt");
        // for(int i = 0; i < quadKeys.size(); i++){
        //     fout << quadKeys.at(i) << "\n";
        // }
        // fout.close();
	}
}