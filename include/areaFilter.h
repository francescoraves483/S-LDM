#ifndef SLDM_AREAFILTER_H
#define SLDM_AREAFILTER_H
extern "C" {
	#include "options.h"
}

class areaFilter {
	private:
		struct options *m_opts_ptr;

	public:
		areaFilter();
		areaFilter(struct options *opts_ptr) : m_opts_ptr(opts_ptr) {}
		void setOptions(struct options *opts_ptr) {m_opts_ptr=opts_ptr;}
		// This function expects the m_opts_ptr pointer to be non-NULL, otherwise it always returns 'false'
		// This function checks if the specified lat and lon value are within the S-LDM coverage area specified
		// by the user and stored inside the options structure (pointed by m_opts_ptr)
		bool isInside(double lat, double lon);
		// This function works as isInside() but returns 'true' only if the specified lat and lon values are located
		// inside the internal area of the S-LDM (thus excluding the external area)
		bool isInsideInternal(double lat, double lon);
};

#endif // SLDM_AREAFILTER_H