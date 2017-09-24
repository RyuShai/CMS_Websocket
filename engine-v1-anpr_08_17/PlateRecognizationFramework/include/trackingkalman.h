#ifndef TRACKING_KALMAN
#define TRACKING_KALMAN

#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>

using namespace cv;
using namespace std;

struct Struct_kalmans
{
    vector<unsigned long> IDs;              // center ID after grouping
    vector<Point2d> centers_out;            // vector of center_out
    vector<Point2d> assigned_traces;// trace of each object
    vector<unsigned long> track_id;         // id of each track
};

    Struct_kalmans process_grouping(const Mat m_cur_frame,vector<Point2d> centers);


#endif