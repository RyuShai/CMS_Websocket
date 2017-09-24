#include "iostream"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <fstream>
#include <ctime>
#include <thread>
#include <chrono>
#include <pqxx/pqxx>
#include <cstring>
#include <string>
#include <limits>
#include <sys/types.h>
#include <sys/stat.h>


#include "ObjectUtils.h"
#include "PlateRecognizator.h"

using namespace std;
using namespace cv;
using namespace pr;

int frameNum = 0;
int plate_num = 0;

std::string cam_id="1";

std::ofstream ofs;
std::string root_folder = "/home/demo-anpr/Desktop/Result";
string plate_folder_result = "/plate/";
string imageVehicle_folder_result = "/vehicle/";

const int Range_Time = 5;
const int MaxMissPlateOfQueue = 15;
const float distance_one_pixcel = 0.00001163; //10m / 480pixcel (tinh theo km)
const int type_plate_vehicle = 5;

vehicle array_result[1001];
bool n_array_result[1001];

queue<FrameData> frameQueue1;

string ObjectUtils::FloatToStr(float tt)
{
	stringstream ss; //convert tt to string
	ss << tt;
	string str = ss.str();
	return str;
}
std::string ObjectUtils::getCurrentDate()
{
	time_t rawtime;
	struct tm *timeinfo;
	char buffer[80];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer, 80, "%Y_%m_%d", timeinfo);
	std::string str(buffer);
	return str;
}


long ObjectUtils::convert_time(string s)
{
	long hh, mm, ss;
	hh = (int)(s[10] - '0') * 10 + (int)(s[11] - '0');
	mm = (int)(s[13] - '0') * 10 + (int)(s[14] - '0');
	ss = (int)(s[16] - '0') * 10 + (int)(s[17] - '0');
	return hh * 3600 + mm * 60 + ss;
}
//===========================find best result=======================
int ObjectUtils::minResult(int x, int y, int z)
{
	int tmp = min(x, y);
	return min(tmp, z);
}

int ObjectUtils::editDist(string str1, string str2, int m, int n)
{
	int dp[m + 1][n + 1];
	for (int i = 0; i <= m; i++)
	{
		for (int j = 0; j <= n; j++)
		{
			if (i == 0)
				dp[i][j] = j; // Min. operations = j
			else if (j == 0)
				dp[i][j] = i; // Min. operations = i
			else if (str1[i - 1] == str2[j - 1])
				dp[i][j] = dp[i - 1][j - 1];
			else
				dp[i][j] = 1 + minResult(dp[i][j - 1],		// Insert
										 dp[i - 1][j],		// Remove
										 dp[i - 1][j - 1]); // Replace
		}
	}
	return dp[m][n];
}

vehicle ObjectUtils::Find_best_result(vehicle plateprocess[maxplateprocess],int num_plates)
{
	vehicle a[num_plates];
	for (int i=0;i<num_plates;i++)
		a[i] = plateprocess[i];
	for (int i=0;i<num_plates;i++)
		a[i].speed = plateprocess[num_plates-1].speed;
	int n = num_plates;
	int b[n + 1];
	memset(b, 0, sizeof(b));
	int min_dist = numeric_limits<int>::max(), vt_min_dist = 0;
	float min_pro = 0;
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
			b[i] += editDist(a[i].vehiclePlate, a[j].vehiclePlate, a[i].vehiclePlate.length(), a[j].vehiclePlate.length());
		if (b[i] < min_dist || (b[i] == min_dist && a[i].pro > min_pro))
		{
			min_dist = b[i];
			vt_min_dist = i;
			min_pro = a[i].pro;
		}
	}
	return a[vt_min_dist];
}

float ObjectUtils::calcBlurriness(const Mat &src)
{
	Mat Gx, Gy;
	Sobel(src, Gx, CV_32F, 1, 0);
	Sobel(src, Gy, CV_32F, 0, 1);
	double normGx = norm(Gx);
	double normGy = norm(Gy);
	double sumSq = normGx * normGx + normGy * normGy;
	return static_cast<float>(1000000. / (sumSq / src.size().area() + 1e-6));
}



bool ObjectUtils::filter_result(vehicle a)
{
    int n = 1000;
    //loai ket qua sau 1 phut
    for (int i=1;i<=n;i++)
        if (array_result[i].vehiclePlate!="@")
        {
            if (a.time - array_result[i].time > 60)
            {
                array_result[i].vehiclePlate="@";
            }
        }
    for (int i=1;i<=n;i++)
        if (array_result[i].vehiclePlate!="@")
            if (editDist(a.vehiclePlate,array_result[i].vehiclePlate,a.vehiclePlate.length(),array_result[i].vehiclePlate.length())<=2)
            {
                array_result[i].time = a.time;
                return false;
            }
    for (int i=1;i<=n;i++)
        if (array_result[i].vehiclePlate=="@")
        {
            array_result[i].vehiclePlate = a.vehiclePlate;
            array_result[i].time = a.time;
            break;
        }
    return true;
}


//end find best result
bool init_array_result = true;
void ObjectUtils::printf_result(vehicle plateprocess[maxplateprocess],int num_plates, Rect cropRect)
{
	this_thread::sleep_for(chrono::milliseconds(1000));
	if (init_array_result==true)
	{
		for (int i=1;i<=1000;i++) 
        	array_result[i].vehiclePlate = "@";
		init_array_result=false;
	}

	vehicle result;
	result = Find_best_result(plateprocess,num_plates);
	//Cap nhat thong tin cho result
	cv::rectangle(result.plate_croped_detail.plate_extended, result.plate_croped_detail.plate_content, cv::Scalar(0, 0, 255), 1, 8, 0);
	
	result.plate = result.plate_croped_detail.plate_extended;

	Rect RectReal = plateprocess[0].square_cascade;
	RectReal.x += cropRect.x;
	RectReal.y += cropRect.y;
	Mat ImageResult = plateprocess[0].vehicleImage.clone();
	cv::rectangle(ImageResult, RectReal, cv::Scalar(0, 255, 0), 2, 8, 0);
	result.speed = ceilf(result.speed);
	putText(ImageResult, FloatToStr(result.speed) + "km/h", cvPoint(RectReal.x, RectReal.y-20), FONT_HERSHEY_COMPLEX_SMALL, 2, cvScalar(0, 0, 0), 20, CV_AA);
	putText(ImageResult, FloatToStr(result.speed) + "km/h", cvPoint(RectReal.x, RectReal.y-20), FONT_HERSHEY_COMPLEX_SMALL, 2, cvScalar(255, 255, 255), 2, CV_AA);

	//TODO: crop vehicle image
    int carX = std::max(RectReal.x - 400, 0);
    int carY = std::max(RectReal.y - 400, 0);
    Rect RectVehicle = cv::Rect(
        carX, carY,
        std::min(RectReal.width + 800, ImageResult.cols - carX-1),
        std::min(RectReal.height + 800, ImageResult.rows - carY-1));
	result.vehicleImage = ImageResult(RectVehicle);
	result.vehicleImageName = "vehicle_"+result.CurrentDateTime+"_"+result.vehiclePlate+".jpg";
	result.link = "plate_"+result.CurrentDateTime+"_"+result.vehiclePlate+".jpg";

	if (result.vehiclePlate != "" && filter_result(result) )
	{
		cout << "result:" << result.vehiclePlate << "\t" << result.pro << "\t" << result.direction << endl;
		std::string folder = root_folder + plate_folder_result + cam_id;
		mkdir(folder.c_str(), ACCESSPERMS);

		std::string folder2 = root_folder + plate_folder_result + cam_id + "/" + getCurrentDate();
		mkdir(folder2.c_str(), ACCESSPERMS);

		std::string folderVehicle = root_folder + imageVehicle_folder_result + cam_id;
		mkdir(folderVehicle.c_str(), ACCESSPERMS);

		std::string folderVehicle2 = root_folder + imageVehicle_folder_result + cam_id + "/" + getCurrentDate();
		mkdir(folderVehicle2.c_str(), ACCESSPERMS);

		std::string speed;
		speed = std::to_string(result.speed);

		AnalyzeCharactor analyzeCharactor;
		analyzeCharactor.analyzePlateText(result.vehiclePlate, result.link, result.vehicleImageName, speed);

		cv::imwrite(root_folder + imageVehicle_folder_result + cam_id + "/" + getCurrentDate() + "/" + result.vehicleImageName, result.vehicleImage);
		cv::imwrite(root_folder + plate_folder_result + cam_id + "/" + getCurrentDate() + "/" + result.link, result.plate);
	}
}



linear_equation ObjectUtils::calculation_line_equa(int x1, int y1, int x2, int y2)
{
	//a*x + b*y + c = 0
	linear_equation line;
    line.a = (float)y1-(float)y2;
    line.b = (float)x2-(float)x1;
    line.c = (float)x1*(float)y2-(float)x2*(float)y1;
	if (line.a==0&&line.b==0)
	{
		line.a = -1; 
        line.b = 0;
        line.c = (float)x1;
	}
    return line;
}

float ObjectUtils::distance_point_to_line(linear_equation line, int x, int y)
{
	float result;
	float x0 = x; float y0=y;
	result = abs(line.a*x0+line.b*y0+line.c)/sqrt(pow(line.a,2)+pow(line.b,2));
	return result;
}

std::string ObjectUtils::getCurrentDateTime()
{
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, 80, "%d-%m-%Y_%I-%M-%S", timeinfo);
    std::string str(buffer);
    return str;
}

