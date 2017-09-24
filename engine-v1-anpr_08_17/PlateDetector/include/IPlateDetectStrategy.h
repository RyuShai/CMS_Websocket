#ifndef IPLATE_DETECT_STRATEGY_H
#define IPLATE_DETECT_STRATEGY_H

#include "PlateRegion.h"
#include "PlateDetectorInputData.h"
#include <vector>

namespace pr{
	class IPlateDetectStrategy{
	public:
		virtual std::vector<PlateRegion> GetPlateRegions(PlateDetectorInputData* data);
	};
}

#endif