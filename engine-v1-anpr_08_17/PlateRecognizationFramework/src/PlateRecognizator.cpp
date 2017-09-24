#include "PlateRecognizator.h"
#include "CascadeTrainingStrategy.h"
#include "opencv2/opencv.hpp"
#include "CascadeTrainingInputData.h"
#include "iostream"

using namespace std;
using namespace pr;

//#define DEBUG;

std::vector<pr::PlateRegion> PlateRecognizator::GetPlateRegions()
{
	// Get Plates Regions
	CascadeTrainingInputData *casData = new CascadeTrainingInputData();
	casData->img = img;
	PlateDetectorInputData *plateDetectorData = (PlateDetectorInputData *)casData;
	plateDetector->SetInputData(plateDetectorData);
	std::vector<PlateRegion> plates = plateDetector->GetPlateRegions();
	casData->img.release();
	img.release();
	return plates;
}

void pr::PlateRecognizator::Init(std::string cascadeFileURL, cv::Size minSize, cv::Size maxSize, double scale, int neighbor)
{
	this->cascadeFileURL = cascadeFileURL;
	InitPlateDetector(minSize, maxSize, scale, neighbor);
}

void pr::PlateRecognizator::InitPlateDetector(cv::Size minSize, cv::Size maxSize, double scale, int neighbor)
{
	std::cout << "Call init plate detector" << std::endl;
	plateDetector = new PlateDetector();
	CascadeTrainingStrategy *casStrategy = new CascadeTrainingStrategy(cascadeFileURL);
	std::cout << minSize << std::endl;
	std::cout << maxSize << std::endl;
	casStrategy->SetMinSize(minSize);
	casStrategy->SetMaxSize(maxSize);
	casStrategy->SetScaleFactor(scale);
	casStrategy->SetMinNeighbor(neighbor);
	IPlateDetectStrategy *strategy = (IPlateDetectStrategy *)casStrategy;
	plateDetector->SetDetectStrategy(strategy);
}

void pr::PlateRecognizator::SetImg(cv::Mat &img)
{
	this->img = img.clone();
}

void pr::PlateRecognizator::ClearImg()
{
	this->img.release();
}

