#ifndef IPLATE_DETECT_H
#define IPLATE_DETECT_H

#include <vector>
#include "PlateDetectorInputData.h"
#include "PlateRegion.h"

namespace pr{
	class IPlateDetect{
	public :
		virtual std::vector<PlateRegion> GetPlateRegions();
	};
}


#endif