#ifndef DEEP_LEARNING_H
#define DEEP_LEARNING_H

#include <opencv2/opencv.hpp>
#include <opencv2/ml.hpp>
#include "keras_model.h"

struct charWithPro {
	float pro;
	char c;
};
struct stringWithPro {
	float pro;
	std::string PlateStr;
	std::string proEachChar;
};

namespace pr {
	class TextReader {
	public:
		keras::KerasModel *m;
		keras::DataChunk *sample;
	public:
		TextReader(std::string dumpfile);
		std::vector< std::vector<std::vector<float > > > mat2data(cv::Mat img);
		charWithPro test_keras2cpp_with_mat(cv::Mat img, int index);
		stringWithPro GetPlateString(std::vector<cv::Rect> charRects, cv::Mat& img);
	};
}

#endif