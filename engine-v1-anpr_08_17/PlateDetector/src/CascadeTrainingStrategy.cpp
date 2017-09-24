#include "CascadeTrainingStrategy.h"
#include "CascadeTrainingInputData.h"
#include "PlateRegion.h"
#include "opencv2/opencv.hpp"

using namespace pr;

std::vector<PlateRegion> CascadeTrainingStrategy::GetPlateRegions(PlateDetectorInputData* data)
{
	std::vector<cv::Rect> plates;
	CascadeTrainingInputData* cdata = (CascadeTrainingInputData*)data;
	//this->classfier.detectMultiScale(cdata->img, plates, 2, 3, 0 | cv::CASCADE_SCALE_IMAGE, minSize, maxSize);	
	this->classfier.detectMultiScale(cdata->img, plates, this->scaleFactor, this->minNeighbor, 0 | cv::CASCADE_SCALE_IMAGE, this->minSize, this->maxSize);
	std::vector<PlateRegion> platesRegions;
	for (int i = 0; i < (int)plates.size(); i++){
		PlateRegion p;
		p.region = plates.at(i);
		p.imgData = cdata->img(p.region).clone();
		platesRegions.push_back(p);		
	}

	return platesRegions;
}

CascadeTrainingStrategy::CascadeTrainingStrategy()
{

}

pr::CascadeTrainingStrategy::CascadeTrainingStrategy(std::string file)
{
	LoadCascadeFile(file);
}

void pr::CascadeTrainingStrategy::LoadCascadeFile(std::string file) // load file cascade
{
	this->cascadeFile = file;
	if (!classfier.load(cascadeFile)){
		std::cout << "Fail to load cascade classifier" << std::endl;
	}
}

void pr::CascadeTrainingStrategy::SetMinSize(cv::Size& _minSize)
{
	this->minSize = _minSize;
	std::cout << "Min size: " << minSize << std::endl;
}

void pr::CascadeTrainingStrategy::SetMaxSize(cv::Size& _maxSize)
{
	this->maxSize = _maxSize;	
	std::cout << "Max size: " << maxSize << std::endl;
}

void pr::CascadeTrainingStrategy::SetScaleFactor(double _scaleFactor)
{
	this->scaleFactor = _scaleFactor;
}

void pr::CascadeTrainingStrategy::SetMinNeighbor(int _minNeighbor)
{
	this->minNeighbor = _minNeighbor;
}

