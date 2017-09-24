#ifndef GROUP_PLATES
#define GROUP_PLATES

#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>
#include "PlateRecognizator.h"
#include "ObjectUtils.h"

using namespace cv;
using namespace std;

struct result_group
{
    vector <string> speed_plates;
    vector <Point2d> rect_plates;
    vector <queue <vehicle>> queuepush;
};

    result_group group_plates(vector <Rect> plates_square, vector <vehicle> plates_process, Mat framefull, Rect cropRect,int frameID, int mode);


#endif