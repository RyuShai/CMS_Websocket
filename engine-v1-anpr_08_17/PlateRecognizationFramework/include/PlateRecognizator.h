#ifndef PLATE_RECOGNIZATOR_H
#define PLATE_RECOGNIZATOR_H

#include "PlateDetector.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>
#include "crop_plate_corner.h"

using namespace cv;
using namespace std;
using namespace pr;

//const input data
const int MaxKindsPlate = 1;
const int MaxFrameForEstimateSpeed = 3;
const int MaxPlateProcessCNN = 4;



struct linear_equation
{
	float a, b, c;
};
struct queue_info
{
	int position_queue_push_vehicle;
	bool print;
	bool free_queue;
	bool Ignore;
};

struct vehicle
{
	int locationX,locationY;//location of palte in croped image
	int distanceX_pointbefore,distanceY_pointbefore;
	int time;
    string link;
    Mat plate;//image plate
    float pro;
    string vehicleImageName;//name of image frame
    Mat vehicleImage;//image frame
    string vehiclePlate;//result
    int frameID;
    float speed;
    string CurrentDateTime; //only for output result to CMS
    int VehicleLocationX,VehicleLocationY; //only for output result to CMS
    int direction;
    linear_equation linear_equations;
    Rect square_cascade;//square cascade of plate
    float distance_to_nearest_plate;
	plate_corner plate_croped_detail;

};



struct QueuePlates
{
	queue <queue <vehicle>> platequeue[MaxKindsPlate];
	mutex platequeueMutex[MaxKindsPlate];
};

struct FrameData
{
	long long frameID;
	cv::Mat frame;
	cv::Mat framefull;
	std::string frameTime;
};
struct PlateData
{
	int frameID;
	cv::Mat frame;
	std::vector<pr::PlateRegion> PlatesSquare[5];
	std::vector<std::string> vehicle_speed[5];
	int direction;
};
struct data_plate
{
	int locationX, locationY;
	std::string currentDateTime;
	cv::Mat imageVehicle;
	cv::Mat plate;
	int frameID;
};
namespace pr
{

typedef std::pair<std::string, data_plate> FileNameWithTime;
typedef std::queue<FileNameWithTime> FileNameWithTimeQueue;
typedef std::chrono::duration<double> ElapsedTime;

// std::queue<vehicle> vehicle_queue[5][10];
// bool check_print_vehicle_queue[5][10];

const int MaxPlateProcess = 4;

class PlateRecognizator
{
  private:
	cv::Mat img;
	std::string cascadeFileURL;
	PlateDetector *plateDetector;

  public:
	void Init(std::string cascadeFileURL, cv::Size minSize, cv::Size maxSize, double scale, int neighbor);
	std::vector<pr::PlateRegion> GetPlateRegions();
	std::string GetResult(cv::Mat &img);
	void SetImg(cv::Mat &img);
	void InitPlateDetector(cv::Size minSize, cv::Size maxSize, double scale, int neighbor);
	void ClearImg();
};
}

#endif