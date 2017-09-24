#ifndef CROP_CHARACTER_RECT
#define CROP_CHARACTER_RECT

#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>

using namespace cv;
using namespace std;

    vector<cv::Rect> getCharacterRect_Square(cv::Mat img_rgb, int mode);

#endif