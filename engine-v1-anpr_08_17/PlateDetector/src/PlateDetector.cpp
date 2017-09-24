#include "PlateDetector.h"
#include "PlateRegion.h"
#include <vector>

using namespace pr;

std::vector<PlateRegion> PlateDetector::GetPlateRegions()
{
	return detectStrategy->GetPlateRegions(this->inputData);
}

void pr::PlateDetector::SetInputData(PlateDetectorInputData* data)
{
	this->inputData = data;
}

void pr::PlateDetector::SetDetectStrategy(IPlateDetectStrategy* strategy)
{
	this->detectStrategy = strategy;
}

pr::PlateDetector::PlateDetector()
{
	
}

