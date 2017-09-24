#ifndef I_O_DATA_H
#define I_O_DATA_H

#include <opencv2/opencv.hpp>
#include <iostream>

enum INPUT_MODE
{
	I_VIDEO,
	I_CAMERA
};

enum DETECT_MODE
{
	CAR_4,
	CAR_5,
	MOTOBIKE_4,
	MOTOBIKE_5
};
namespace pr
{

class IOData
{
  public:
	static std::string GetLinkURL();
	static INPUT_MODE GetInputMode();
	static DETECT_MODE GetDetectMode();
	static std::string GetCarPlateCascade(int mode);
	static std::string GetMotobikePlateCascade();
	static std::string GetCarPlatelogistic();
	static std::string GetMotobikePlatelogistic();
	static std::string GetCongfigData(std::string key);
};
}

#endif