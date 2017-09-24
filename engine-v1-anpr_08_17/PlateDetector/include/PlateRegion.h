#ifndef PLATE_REGION_H
#define PLATE_REGION_H

#include "opencv2/opencv.hpp"

namespace pr{
	class PlateRegion{
	public:
		cv::Rect region;		
		cv::Mat imgData;
	};
}
#endif