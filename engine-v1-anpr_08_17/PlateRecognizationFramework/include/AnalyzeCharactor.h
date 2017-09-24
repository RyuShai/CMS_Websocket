#ifndef ANALYZE_CHARACTOR_DATA_H
#define ANALYZE_CHARACTOR_DATA_H

#include <iostream> 

using namespace cv; 
using namespace std; 

namespace pr {
class AnalyzeCharactor {
private:
std::string test;

public:
void writeDataToDB(std::string plate, char * province, std::string image, std::string vehicleImage,std::string speed, char * type); 
void filterProvince(std::string fullText, std::string text, std::string image, std::string vehicleImage,std::string speed, char * type); 
void analyzePlateText(std::string fullText, std::string image, std::string vehicleImage,std::string speed); 
}; 
}
#endif
