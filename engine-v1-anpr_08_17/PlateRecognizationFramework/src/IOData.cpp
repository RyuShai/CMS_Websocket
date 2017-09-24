#include "iostream"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <fstream> 
#include <ctime>

#include "IOData.h"
using namespace pr;
using namespace std;

string IOData::GetLinkURL()
{
	string data;
	string line;
	string url;
	ifstream config;
	config.open("../data/Config.txt");
	while (std::getline(config, line))
	{
		data += line + "|";
	}
	config.close();
	std::size_t start = data.find("url:");
	url = data.substr(start + 4);
	std::size_t end = url.find("|");
	url = url.substr(0,end);
	return url;
}

string IOData::GetCarPlateCascade(int mode)
{
	string data;
	string line;
	string url;
	ifstream config;
	config.open("../data/Config.txt");
	while (std::getline(config, line))
	{
		data += line + "|";
	}
	config.close();
	std::size_t start = data.find("car_cascade:");
	url = data.substr(start + 12);
	std::size_t end = url.find("|");
	url = url.substr(0, end);
	return url;
}

string IOData::GetMotobikePlateCascade()
{
	string data;
	string line;
	string url;
	ifstream config;
	config.open("../data/Config.txt");
	while (std::getline(config, line))
	{
		data += line + "|";
	}
	config.close();
	std::size_t start = data.find("motobike_cascade:");
	url = data.substr(start + 17);
	std::size_t end = url.find("|");
	url = url.substr(0, end);
	return url;
}

string IOData::GetCarPlatelogistic()
{
	string data;
	string line;
	string url;
	ifstream config;
	config.open("../data/Config.txt");
	while (std::getline(config, line))
	{
		data += line + "|";
	}
	config.close();
	std::size_t start = data.find("car_logistic:");
	url = data.substr(start + 13);
	std::size_t end = url.find("|");
	url = url.substr(0, end);
	return url;
}

string IOData::GetMotobikePlatelogistic()
{
	string data;
	string line;
	string url;
	ifstream config;
	config.open("../data/Config.txt");
	while (std::getline(config, line))
	{
		data += line + "|";
	}
	config.close();
	std::size_t start = data.find("motobike_logistic:");
	url = data.substr(start + 18);
	std::size_t end = url.find("|");
	url = url.substr(0, end);
	return url;
}

INPUT_MODE IOData::GetInputMode()
{
	string data;
	string line;
	string mode;
	INPUT_MODE enumMode;
	ifstream config;
	config.open("../data/Config.txt");
	while (std::getline(config, line))
	{
		data += line + "|";
	}
	config.close();
	std::size_t start = data.find("input_mode:");
	mode = data.substr(start + 11);
	std::size_t end = mode.find("|");
	mode = mode.substr(0, end);
	if (mode == "video") enumMode = I_VIDEO;
	else enumMode = I_CAMERA;
	return enumMode;
}

DETECT_MODE IOData::GetDetectMode()
{
	string data;
	string line;
	string mode;
	DETECT_MODE enumMode;
	ifstream config;
	config.open("../data/Config.txt");
	while (std::getline(config, line))
	{
		data += line + "|";
	}
	config.close();
	std::size_t start = data.find("detect_mode:");
	mode = data.substr(start + 12);
	std::size_t end = mode.find("|");
	mode = mode.substr(0, end);
	if (mode == "motobike_4") enumMode = MOTOBIKE_4;
	else if (mode == "motobike_5") enumMode = MOTOBIKE_5;
	else if (mode == "car_4") enumMode = CAR_4;
	else enumMode = CAR_5;
	return enumMode;
}
std::string IOData::GetCongfigData(std::string key){
	string data;
	string line;
	string url;
	ifstream config;
	config.open("../data/Config.txt");
	while (std::getline(config, line))
	{
		data += line + "|";
	}
	config.close();
	std::size_t start = data.find(key);
	url = data.substr(start + key.length());
	std::size_t end = url.find("|");
	url = url.substr(0, end);
	return url;
}