#include "areaFilter.h"

areaFilter::areaFilter() {
	m_opts_ptr=nullptr;
}

bool areaFilter::isInside(double lat, double lon) {
	if(m_opts_ptr==nullptr) {
		return false;
	}

	if(lat >= m_opts_ptr->min_lat-m_opts_ptr->ext_lat_factor && lat <= m_opts_ptr->max_lat+m_opts_ptr->ext_lat_factor &&
		lon >= m_opts_ptr->min_lon-m_opts_ptr->ext_lon_factor && lon <= m_opts_ptr->max_lon+m_opts_ptr->ext_lon_factor) {
		return true;
	}

	return false;
}

bool areaFilter::isInsideInternal(double lat, double lon) {
	if(m_opts_ptr==nullptr) {
		return false;
	}

	if(lat >= m_opts_ptr->min_lat && lat <= m_opts_ptr->max_lat &&
		lon >= m_opts_ptr->min_lon && lon <= m_opts_ptr->max_lon) {
		return true;
	}

	return false;
}