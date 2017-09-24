#include "TextReader.h"
#include <opencv2/opencv.hpp>
#include <opencv2/ml.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"


using namespace pr;
using namespace std;
using namespace keras;
using namespace cv;

char ch[33] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','G','H','K','L','M','N','P','R','S','T','U','V','X','Y','Z','_'};

//"../data/DeepLearningdata/cnn-2pool-20170221.dump"
TextReader::TextReader(string dumpfile)
{
	m = new KerasModel(dumpfile, true);
	sample = new DataChunk2D();
}
std::vector< std::vector<std::vector<float > > > TextReader::mat2data(cv::Mat img) {
	cv::Mat img_resize;
	cv::resize(img, img_resize, cv::Size(14, 28));
	vector< vector<vector<float > > > data;
	vector<vector<float> > mat;
	for (int i = 0; i < img_resize.rows; i++) {
		vector<float> row;
		for (int j = 0; j < img_resize.cols; j++) {
			row.push_back((float)img_resize.at<uchar>(i, j) / 255);
		}
		mat.push_back(row);
	}
	data.push_back(mat);
	return data;
}

charWithPro TextReader::test_keras2cpp_with_mat(cv::Mat img, int mode) {
	charWithPro cwp;
	if (img.channels() != 1) 
		cv::cvtColor(img, img, CV_BGR2GRAY);
	sample->set_data(mat2data(img));
	char c;
	float max = 0.0;
	//mode: 1 = Num + Word + Negative; 	2 = Num + Word; 
	//		3 = Num + Negative; 		4 = Num; 
	//		5 = Word + Negative; 		6 = Word; 
	auto res = m->compute_output(sample);

	if (mode == 1) //Num + Word + Negative;
	{
		for (int k = 0; k < (int)res.size(); k++) {
			if (res[k] > max) {
			max = res[k];
			c = ch[k];
			}
		}
	}
	if (mode == 2) //Num + Word; 
	{
		for (int k = 0; k < (int)res.size()-1; k++) {
			if (res[k] > max) {
			max = res[k];
			c = ch[k];
			}
		}
	}
	if (mode == 3) //Num + Negative;
	{
		for (int k = 0; k < 10; k++) {
			if (res[k] > max) {
				max = res[k];
				c = ch[k];
			}
		}
		int k=(int)res.size()-1;
		if (res[k] > max) {
			max = res[k];
			c = ch[k];
		}
	}
	if (mode == 4) //Num; 
	{
		for (int k = 0; k < 10; k++) {
			if (res[k] > max) {
			max = res[k];
			c = ch[k];
			}
		}
	}
	if (mode == 5) //Word + Negative; 
	{
		for (int k = 10; k < (int)res.size(); k++) {
			if (res[k] > max) {
			max = res[k];
			c = ch[k];
			}
		}
	}
	if (mode == 6) //Word; 
	{
		for (int k = 10; k < (int)res.size()-1; k++) {
			if (res[k] > max) {
			max = res[k];
			c = ch[k];
			}
		}
	}

	//delete sample;
	cwp.c = c;
	cwp.pro = max;
	return cwp;
}

stringWithPro TextReader::GetPlateString(std::vector<cv::Rect> charRects, cv::Mat& img) {
	stringWithPro swp;
	std::string plateStr = "";
	float pro = 0;
	string proEachChar = "";
	int location_minus=100;
	for (int i = 0; i < (int)charRects.size(); i++)
	{
		Rect r1 = charRects[i];
		if (r1.x==0 && r1.y==0 && r1.width==0 && r1.height==0) //tim vi tri dau "-"
			location_minus = i;
	}
	//mode: 1 = Num + Word + Negative; 	2 = Num + Word; 
	//		3 = Num + Negative; 		4 = Num; 
	//		5 = Word + Negative; 		6 = Word; 
	charWithPro cwp;
	//keras cnn
	int n = (int)charRects.size();
	int k=1;
	for (int i = 0; i < n; i++)
	{
		if (i==location_minus)
		{
			plateStr+="-";
			pro+=1;
			proEachChar += to_string(1.00000) + "_";
		}
		else//binh thuong
		{
			if (k!=3 && k!=4)
				cwp = test_keras2cpp_with_mat(img(charRects[i]), 3);
			else 
				if (k==3)
					cwp = test_keras2cpp_with_mat(img(charRects[i]), 5);
				else if (k==4) 
				{
					if (cwp.c == 'A' || cwp.c == 'M' || cwp.c == 'L' || cwp.c == 'N')
						cwp = test_keras2cpp_with_mat(img(charRects[i]), 1);
					else 
					cwp = test_keras2cpp_with_mat(img(charRects[i]), 3);
				}
			if (cwp.c!='_')
			{
				plateStr += cwp.c;
				pro += cwp.pro;
				proEachChar += to_string(cwp.pro) + "_";
			}
			if (cwp.c!='_')
				k++;
		}
	}
	swp.PlateStr = "";
	if (plateStr.length()>=8)
	{
		swp.PlateStr = plateStr;
		swp.pro = pro/(int)charRects.size();
		swp.proEachChar = proEachChar;
	}
	return swp;
}

/*stringWithPro TextReader::GetPlateString(std::vector<cv::Rect> charRects, cv::Mat& img) {
	stringWithPro swp;
	std::string plateStr = "";
	float pro = 0;
	string proEachChar = "";
	int location_minus=100;
	for (int i = 0; i < (int)charRects.size(); i++)
	{
		Rect r1 = charRects[i];
		if (r1.x==0 && r1.y==0 && r1.width==0 && r1.height==0) //tim vi tri dau "-"
			location_minus = i;
	}
	//mode: 1 = Num + Word + Negative; 	2 = Num + Word; 
	//		3 = Num + Negative; 		4 = Num; 
	//		5 = Word + Negative; 		6 = Word; 
	int n = (int)charRects.size();
	charWithPro cwp;
	if (n<7) //isn't plate
	{
		swp.PlateStr = "";
		swp.pro = 0;
		swp.proEachChar = "";
		return swp;
	}
	else
	{
		//create array mode
		int mode[n];
		if (location_minus==100) //long plate
		{
			mode[0] = 3;
			mode[1] = 4;
			for (int i=2;i<=4;i++)
				mode [i] = 2;
			for (int i=5;i<n-1;i++)
				mode [i] = 4;
			mode[n-1] = 3;
		}
		else //square plate
		{
			mode[0]=3;
			mode[1]=4;
			for (int i=2;i<location_minus-1;i++)
				mode[i]=2;
			mode[location_minus-1]=1;
			mode[location_minus+1]=3;
			for (int i=location_minus+2;i<n-1;i++)
				mode[i]=4;
			mode[n-1]=3;
		}
		//keras cnn
		for (int i = 0; i < n; i++)
		{
			if (i==location_minus)
			{
				plateStr+="-";
				pro+=1;
				proEachChar += to_string(1.00000) + "_";
			}
			else//binh thuong
			{
				cwp = test_keras2cpp_with_mat(img(charRects[i]), mode[i]);
				//if (cwp.c!='_')
				{
					plateStr += cwp.c;
					pro += cwp.pro;
					proEachChar += to_string(cwp.pro) + "_";
				}
			}
		}
	}
	swp.PlateStr = plateStr;
	swp.pro = pro/(int)charRects.size();
	swp.proEachChar = proEachChar;
	return swp;
}*/