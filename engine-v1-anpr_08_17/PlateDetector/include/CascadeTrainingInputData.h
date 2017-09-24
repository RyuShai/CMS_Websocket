#ifndef CASCADE_TRAINING_INPUT_DATA_H
#define CASCADE_TRAINING_INPUT_DATA_H

#include "PlateDetectorInputData.h"
#include "opencv2/opencv.hpp"

namespace pr{
	class CascadeTrainingInputData : PlateDetectorInputData{
	public:
		cv::Mat img;
	};
}

#endif