#ifndef CAR_TEXT_ISOLATION_H
#define CAR_TEXT_ISOLATION_H

#include <opencv2/opencv.hpp>

using namespace cv;

namespace pr{
	class CarTextIsolation{

	public:
		std::vector<cv::Rect> getCharacterRect(cv::Mat& img, int mode);
		std::vector<cv::Rect> getCharacterRect_Horizol(cv::Mat& img, int mode);
		bool detectlinecutchar(Point p1, Point p2, Mat img);
		bool detectlinecutcharHorizol(Point p1, Point p2, Mat img);
		bool IsValidate(Rect r, Mat& img);
		cv::Mat CropPlate(cv::Mat& src,float& rate_crop_4point, Rect& Rect_small_CropPlate);
		cv::Mat CropPlate_Horizol(cv::Mat& src,float& rate_crop_4point);
	};
}


#endif