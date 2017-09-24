#pragma once
#include "Kalman.h"
#include "HungarianAlg.h"
#include "opencv2/opencv.hpp"
#include <iostream>
#include <vector>
using namespace cv;
using namespace std;

class QTrack
{
public:
	vector<Point2d> trace;
	static size_t NextTrackID;
	size_t track_id;
	size_t skipped_frames; 
	size_t crossBorder;
	Point2d prediction;
	TKalmanFilter* KF;
	QTrack(Point2f p, float dt, float Accel_noise_mag);
	~QTrack();
};


class QTrackers
{
public:
	

	float dt; 

	float Accel_noise_mag;

	double dist_thres;
	int maximum_allowed_skipped_frames;
	int max_trace_length;

	vector<QTrack*> tracks;
	vector<int> Update(vector<Point2d>& detections, Point2d frameSize);
	QTrackers(float _dt, float _Accel_noise_mag, double _dist_thres=60, int _maximum_allowed_skipped_frames=10,int _max_trace_length=10);
	~QTrackers(void);
};

