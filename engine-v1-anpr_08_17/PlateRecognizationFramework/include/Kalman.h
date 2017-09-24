#pragma once
#include "opencv2/opencv.hpp"
#include <opencv/cv.h>
using namespace cv;
using namespace std;

class TKalmanFilter
{
public:
	KalmanFilter* kalman;
	double deltatime;
	Point2f LastResult;
	TKalmanFilter(Point2f p,float dt,float Accel_noise_mag);
	~TKalmanFilter();
	Point2f GetPrediction();
	Point2f Update(Point2f p, bool DataCorrect);
};
