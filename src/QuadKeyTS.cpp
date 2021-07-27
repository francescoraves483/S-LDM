#include <string>
#include <iostream>
#include <sstream>
#include <cmath>
#include <numeric>
#include <fstream>
#include <algorithm>
#include <limits>
#include <iomanip>
#include "QuadKeyTS.h"
#include "utils.h"

namespace QuadKeys
{
	std::string 
	QuadKeyTS::getQuadKeyFilter(double min_latitude, double min_longitude, double max_latitude, double max_longitude,bool *cachefilefound) {
		std::string line;
		bool cache_file_found = false;
		std::ifstream ifile("cachefile.sldmc");
		std::vector<std::string> fromfile;
		std::vector<std::string> quadKeys;
		std::string filter_string="";
		
		// uint64_t bf = 0.0,af = 0.0;

		// if(m_logfile_name!="") {
		// 	bf=get_timestamp_ns();
		// }

		if(ifile.is_open()) {
			std::cout<<"[QuadKeyTS - GetQuadKeyFilter] Cache file available: reading the parameters..."<< std::endl;

			while(getline(ifile, line)) {
				fromfile.push_back(line);
			}

			double minlatff = stod(fromfile.at(0));
			double maxlatff = stod(fromfile.at(1));
			double minlonff = stod(fromfile.at(2));
			double maxlonff = stod(fromfile.at(3));

			/*fprintf(stdout,"From File we get max_latitude: %.40lf\n",maxlatff);
			fprintf(stdout,"Actual max_latitude parameter%.40lf\n",max_latitude);
			fprintf(stdout,"From File we get min_latitude: %.40lf\n",minlatff);
			fprintf(stdout,"Actual min_latitude parameter%.40lf\n",min_latitude);*/

			if(doublecomp(minlatff, min_latitude) && doublecomp(maxlatff, max_latitude) && doublecomp(minlonff, min_longitude) && doublecomp(maxlonff, max_longitude) && fromfile.size() > 4){
				cache_file_found = true;
			}
		} else {
			std::cout<<"[QuadKeyTS - GetQuadKeyFilter] No cache file found!"<<std::endl;
		}

		ifile.close();

		if(cache_file_found == false) {
			std::ofstream ofile("cachefile.sldmc");

			std::cout<<"[QuadKeyTS - GetQuadKeyFilter] New coordinates: recomputing quadkeys..."<<std::endl;
			std::cout<<"[QuadKeyTS - GetQuadKeyFilter] Maximum level of details: "<<m_levelOfDetail<<std::endl;
			std::cout<<"[QuadKeyTS - GetQuadKeyFilter] Lat/Lon range scanning accuracy: "<<m_latlon_variation<<std::endl;

			// Here we get the vector containing all the quadkeys in the range at a given level of detail
			quadKeys = LatLonToQuadKeyRange(min_latitude, max_latitude, min_longitude, max_longitude);

			// Add the range information to the cache file
			if(ofile.is_open()) {
				ofile << std::fixed << std::setprecision(6) << min_latitude << "\n" << max_latitude << "\n" << min_longitude << "\n" << max_longitude << "\n";
			}

			// Quadkeys unifier algorithm
			// unifyQuadkeys(quadKeys);
			quadKeys=unifyQuadkeys2(quadKeys);
			checkdim(quadKeys);

			// Write the computed Quadkeys to the cache file
			std::ofstream file;
			if(ofile.is_open()) {
				for(size_t i = 0; i < quadKeys.size(); i++){
					ofile << quadKeys.at(i) << "\n";
				}
			}

			ofile.close();

			std::cout<<"[QuadKeyTS - GetQuadKeyFilter] Finished: Quadkey cache file created."<<std::endl;

			// Here we create a string to pass to the filter (SQL like)
			for(size_t i = 0; i < quadKeys.size(); i++) {
				filter_string.insert(filter_string.length(), "quadkeys LIKE ''");
				//l = s.length() - 1;
				filter_string.insert(filter_string.length() - 1, quadKeys.at(i));
				filter_string.insert(filter_string.length() - 1, "%");
				if(i < quadKeys.size()-1){
					filter_string.insert(filter_string.length(), " OR ");
				}
			}
		} else {
			std::cout<<"[QuadKeyTS - GetQuadKeyFilter] Filter setup from a cache file... "<<std::endl;

			for(size_t i = 4; i < fromfile.size(); i++) {
				filter_string.insert(filter_string.length(), "quadkeys LIKE ''");
				//l = s.length() - 1;
				filter_string.insert(filter_string.length() - 1, fromfile.at(i));
				filter_string.insert(filter_string.length() - 1, "%");
				if(i < fromfile.size()-1){
					filter_string.insert(filter_string.length(), " OR ");
				}
			}
		}

		if(cachefilefound!=nullptr) {
			*cachefilefound=cache_file_found;
		}

		return filter_string;
	}

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
			m_latlon_variation = 0.003;
			break;
		case 15:
			m_latlon_variation = 0.007;
			break;
		case 14:
			m_latlon_variation = 0.015;
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

		// Round to the second decimal digit to avoid any issue when computing the Quadkeys and increasing the lat and lon
		// of a value (i.e., m_latlon_variation) which is specified up to three digits after the decimal point
		// If we keep more digits in the starting (and final) values, these will always be kept as an "offset" to each computed
		// value (e.g., if min_latitude=46.121228 and m_latlon_variation=0.004, we will consider 46.121228 then 46.125228 then 46.129228
		// and so on, always bringing with us the 0.000228 "offset", which sometimes seems to cause issues in the Quadkeys computation,
		// leading to a few skipped Quadkeys in some specific circumstances)
		// Round the minimum values with floor() and the maximum values with ceil() to avoid "losing" even any small portion
		// of the area covered by the S-LDM when computing the Quadkeys covering that area
		min_latitude = std::floor(min_latitude*100.0)/100.0;
		max_latitude = std::ceil(max_latitude*100.0)/100.0;
		min_longitude = std::floor(min_longitude*100.0)/100.0;
		max_longitude = std::ceil(max_longitude*100.0)/100.0;

		std::string quadKey;
		std::vector<std::string> v = {};

		// Starting scan the lan_lon using the private attribute m_latlon_variation
		for(double j = min_latitude; j <= max_latitude; j+=m_latlon_variation) {
			for(double k = min_longitude; k <= max_longitude; k+=m_latlon_variation) {

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
	
	std::vector<std::string> QuadKeyTS::unifyQuadkeys2(std::vector<std::string> quadKeys) {
		std::vector<std::string> vf = {};
		bool reduction_occurred = false;

		// std::sort(std::begin(quadKeys), std::end(quadKeys));

		// Sort the array (this function requires the vector to be sorted alphabetically)
		std::sort(std::begin(quadKeys),std::end(quadKeys));

		for(int v=0;v<m_levelOfDetail;v++) {
			// std::sort(std::begin(quadKeys), 
			// 	std::end(quadKeys), 
			// 	[](const std::string& lhs, const std::string& rhs) {return std::make_tuple(lhs.size(), std::stol(lhs)) < std::make_tuple(rhs.size(), std::stol(rhs));});

			// std::sort(std::begin(quadKeys),std::end(quadKeys));

			// for(int bh=0;bh<quadKeys.size();bh++) {
			// 	std::cout << quadKeys[bh] << std::endl;
			// }

			// std::cout << "v = " << v << " - m_levelOfDetail = " << m_levelOfDetail << " reduction_occurred = " << reduction_occurred << " quadKeys.size() = " << quadKeys.size() << std::endl;
			
			reduction_occurred = false;

			for(int i=0;i<quadKeys.size();i++) {
				std::string main_part = quadKeys.at(i).substr(0,quadKeys.at(i).length()-1);

				if(i<=quadKeys.size()-4 && quadKeys.at(i).back()=='0' &&
					main_part == quadKeys.at(i+1).substr(0,quadKeys.at(i+1).length()-1) && quadKeys.at(i+1).back()=='1' &&
					main_part == quadKeys.at(i+2).substr(0,quadKeys.at(i+2).length()-1) && quadKeys.at(i+2).back()=='2' &&
					main_part == quadKeys.at(i+3).substr(0,quadKeys.at(i+3).length()-1)&& quadKeys.at(i+3).back()=='3') {

					// std::cout << "main_part: " << main_part << "| 1: " << quadKeys.at(i+1) << "| 2: " << quadKeys.at(i+2) << "| 3: " << quadKeys.at(i+3) << std::endl;
					vf.push_back(main_part);
					i+=3;

					reduction_occurred = true;
				} else {
					vf.push_back(quadKeys.at(i));
				}
	 		}

	 		// std::cout << "reduction_occurred = " << reduction_occurred << std::endl;

	 		if(reduction_occurred == false) {
	 			break;
	 		}

	 		quadKeys = std::move(vf);
	 		// vf.clear();
	 	}

 		// vf.erase(std::unique(vf.begin(),vf.end()),vf.end());

	 	// Sort the final vector by the length of the strings (i.e., the length of the Quadkeys), to place first the shorter Quadkeys, corresponding
	 	// to larger areas, in which it should be more probable to have a greater number of vehicles (i.e, more messages are probably coming from
	 	// these areas)
	 	// As the AMQP 1.0 filter is an OR between all the Quadkeys, having first the Quadkeys in which it is more probable to have a greater number 
	 	// of vehicles may slightly improve the broker-side filtering performance (as the match may be found earlier if the broker is "scanning"
	 	// all the "OR-ed" Quadkeys in the filter in a sequential way)
 		std::sort(std::begin(quadKeys),std::end(quadKeys),[](const std::string &lhs, const std::string &rhs) {return lhs.size() < rhs.size();});

 		return quadKeys;
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

	// This function is in charge of reducing the size of the filter string which will be passed to the AMQP broker to filter the messages depending on the Quadkeys
	// This function takes as argument a reference to an array of Quadkeys and modifies it in order not to exceed max_length bytes
	// The compression is performed by removing one character from the longest Quadkey strings, thus increasing the total area covered by all the Quadkeys
	// The Area Filter and Decoder modules will therefore "work a bit more" when we compress more, because the Quadkey area will be a bit larger and more messages 
	// will need to be decoded and passed to the Area Filter
	// For the time being, the maximum filter string size is set to 100000 B, which seems to be a good compromise between the size of the filter and the amount
	// of resulting compression when the area is very large
	void QuadKeyTS::checkdim(std::vector<std::string> &quadKeys) {
		std::vector<std::string> vf = quadKeys;
		const int max_length = 100000; // 131072; // 222; maximum length in byte to transfer infos to the broker
		unsigned int l = m_levelOfDetail; // 5;
		//int vec_len = vf.size();

		// With std::accumulate we compite the size of the resulting filter string starting from all the Quadkeys
		// "+21" takes into account the other characters which are needed in the final SQL-like filter, including "%" and "OR"
		std::cout<<"[QUADKEYS] Checking dimensions: "<<std::accumulate(vf.begin(), vf.end(), -4, [](int sum, const std::string& elem) {return sum + elem.size() + 21;}) + 180<<std::endl;
		
		if(std::accumulate(vf.begin(), vf.end(), -4, [](int sum, const std::string& elem) {return sum + elem.size() + 21;}) + 180 > (max_length)){
			std::cout<<"[QUADKEYS] Dimensions exceeded, resizing:"<<std::endl;
			if(std::accumulate(vf.begin(), vf.end(), -4, [](int sum, const std::string& elem) {return sum + elem.size() + 21;}) + 180 > 2*(max_length)){
				std::cout << "[QUADKEYS] Warning: Too Large Area, the process can take a while.. (hint: try reducing the area or the level of detail)" << std::endl;
			}	
		}

		while(std::accumulate(vf.begin(), vf.end(), -4, [](int sum, const std::string& elem) {return sum + elem.size() + 21;}) + 180 >= max_length ) {
	   	 	for (size_t j = 0; j < vf.size(); j++) {
				if(vf.at(j).size() ==  l) {
					//std::cout<<"Level of detail: "<<m_levelOfDetail<< " " << l<< " " <<vf.at(j).substr(0,l-1)<<std::endl;
					vf.push_back(vf.at(j).substr(0,l-1));
					auto it = vf.begin();
					vf.erase(it + j);
					break;
				}
				else if(j == vf.size() - 1){						
				    l--;
					break;
				}
			}
				    
			if(l<1) {
				break;
			}

			// Removing duplicates resulting from the compression due to the removal of the last character from the longest strings
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
		}

		std::cout<<"[QUADKEYS] Final dimension: "<< std::accumulate(vf.begin(), vf.end(), -4, [](int sum, const std::string& elem) {return sum + elem.size() + 21;}) + 180 <<std::endl;
		
		// QuadKeyTS::unifyQuadkeys(vf);
		
		quadKeys = vf;

		// This is commented as it is intended only for debug purposes
		// lastvec.txt contains the final Quadkey vector content after the size reduction phase (if it happens)
		// std::ofstream fout("lastvec.txt");
		// for(size_t i = 0; i < quadKeys.size(); i++){
		// 	fout << quadKeys.at(i) << "\n";
		// }
		// fout.close();
	}
}