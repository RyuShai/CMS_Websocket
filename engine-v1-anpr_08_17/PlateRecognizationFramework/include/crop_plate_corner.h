#ifndef CROP_PLATE_CORNER
#define CROP_PLATE_CORNER

#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>

using namespace cv;
using namespace std;

struct plate_corner
{
    Point cn1;
    Point cn2;
    Point cn3;
    Point cn4;
    float plate_score;
    Mat plate_extended;
    Rect plate_content;
    bool isplate;
};

    plate_corner crop_plate_corner(Mat img,int mode);


#endif