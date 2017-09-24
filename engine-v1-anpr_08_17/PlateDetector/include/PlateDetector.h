#ifndef PLATE_DETECTOR_H
#define PLATE_DETECTOR_H

#include "IPlateDetect.h"
#include "IPlateDetectStrategy.h"
#include "PlateDetectorInputData.h"

namespace pr{
	class PlateDetector: IPlateDetect{
	private:
		IPlateDetectStrategy* detectStrategy;
		PlateDetectorInputData* inputData;
	public:
		PlateDetector();
		std::vector<PlateRegion> GetPlateRegions() override;
		void SetInputData(PlateDetectorInputData* data);
		void SetDetectStrategy(IPlateDetectStrategy* strategy);
	};
}

#endif