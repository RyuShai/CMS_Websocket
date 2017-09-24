#ifndef ESTIMATE_SQUARE_CASCADE
#define ESTIMATE_SQUARE_CASCADE

#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>
#include "PlateRecognizator.h"

using namespace cv;
using namespace std;

struct cascade_input
{
    int carSquareMin_x;
    int carSquareMin_y;
    int carSquareMax_x;
    int carSquareMax_y;
    float detectScale;
	
	int carLongMin_x;
    int carLongMin_y;
    int carLongMax_x;
    int carLongMax_y;
    float detectScaleLong;
};



cascade_input estimate_square_cascade(queue<long> car_estimate_square_cascade);


#endif