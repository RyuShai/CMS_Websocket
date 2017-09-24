#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <numeric>
#include <string>
#include <sstream>
#include <functional>
#include <queue>
#include "crop_character_rect.h"
using namespace cv;
using namespace std;
struct space 
{
int start_point;
int end_point;
int mid_point;
};
int check_character(Mat img,int left, int right, int up, int down)
{

int result;
int check; //number of blank line
int max;
int start,end;
result=1;
check=0;
//check 1: check so hang co qua it pixel den
for (int i=up;i<down;i++)
	{
	max=0;
	for (int j=left;j<right;j++)
		{
		if ((img.at<uchar>(i, j)<100))
			{
			max++;
			}
		}
	if (max<4)
		{
		check++;
		}
	}
if (check>0.5*(down-up)) //check gom so hang co qua it pixel den
	{
	result=0;
	}
//check 2: check so hang co toan pixel trang 
check =0;
/*for (int i=up;i<down;i++)
	{
	max=0;
	for (int j=left;j<right;j++)
		{
		if ((img.at<uchar>(i, j)<150))
			{
			max=1;
			break;
			}
		}
	if (max==0)
		{
		check++;
		}
	}*
//printf("check =%d\n",check);
if (check>0.3*(down-up)) //check la so hang gom toan pixel trang
	{
	result=0;
	}*/
//printf("left=%d, right=%d,up=%d,down=%d result=%d\n",left,right,up,down,result);
//result=1;
return (result);
}
void bwareaopen(cv::Mat& im, double size)
{
    // Only accept CV_8UC1
    if (im.channels() != 1 || im.type() != CV_8U)
        return;

  //  cv::bitwise_not(im, im);
    // Find all contours
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(im.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

    for (int i = 0; i < contours.size(); i++)
    {
        // Calculate contour area
        double area = cv::contourArea(contours[i]);

        // Remove small objects by drawing the contour with black color
        if (area > 0 && area <= size)
            cv::drawContours(im, contours, i, CV_RGB(0, 0, 0), -1);
    }
  //  cv::bitwise_not(im, im);
}
std::vector<cv::Rect> getCharacterRect_Square(cv::Mat img_rgb, int mode)
{
	std::vector<cv::Rect> charRegions;
	Mat img, img_t1,img_t2,img_bw; // img: gray image; img_t1: binary image
	cvtColor(img_rgb, img,CV_RGB2GRAY);
	
	int height_ori=img.size().height; // height of the original image
	int width_ori=img.size().width;	// width of the original image
	//prepare another binay image for filter out the non-character REC
	img_t2=img;
	cv::resize(img_t2, img_t2, cv::Size(216,160), 0, 0, CV_INTER_LINEAR);
	equalizeHist( img_t2, img_t2);
	bitwise_not(img_t2,img_t2);
	adaptiveThreshold(img_t2, img_bw, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 21,-15);
	bwareaopen(img_bw,200);
	bitwise_not(img_bw,img_bw);
	//---Pre processing the image with resize, equalize---
	cv::resize(img, img, cv::Size(216,160), 0, 0, CV_INTER_LINEAR);
	equalizeHist( img, img);
	bitwise_not(img,img);
	adaptiveThreshold(img, img_t1, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 21,1);
	bwareaopen(img_t1,500); // Delete small object
	bitwise_not(img_t1,img_t1);
	//--------------------Begin to process the image------------------------------------------
	int height=img.size().height;  // height of the resized image (160)
	int width=img.size().width;		// widthe of the resized image (216)
	int hist_h[300]; // projection of all pixel values on the horizontal direction
	int hist_hh[300];// sum over 3 pixels of hist_h;
	int project[300]; // projection of all pixel values on the vertical direction 
	float cx=0.7;
	float cw=0.83;
	int g_max=0; // the maximum value of projection, this is used as a threshold to define if a pixel is lie on a space or a character
	for (int i=1;i<300;i++)
			{
			hist_h[i]=0;
			hist_hh[i]=0;
			}
	for (int i=(floor(height/2)-16);i<(floor(height/2)+16);i++)
		{
		for (int j=0;j<width;j++)
			{
			hist_h[i]=hist_h[i]+img_t1.at<uchar>(i, j);
			}
		}
	for (int i=(floor(height/2)-15);i<(floor(height/2)+15);i++)
		{
		hist_hh[i]=hist_h[i-1]+hist_h[i]+hist_h[i+1];
		}
	int max,index_line;  // index_line is the position where we seperate the plate into upper and lower regions
	max=0;
	index_line=0;
	for (int i=(floor(height/2)-15);i<(floor(height/2)+15);i++)
		{
		if (max<hist_hh[i])
			{
			max=hist_hh[i];
			index_line=i;
			}
		}
	int index_line_ori; // the index_line in the original image
	index_line_ori=floor (index_line*height_ori/height);
	float ratio_width=(float) width_ori/width;
	//---------------------------crop character of the upper regions---------------------------------------------------------------------------------
		for (int i=0;i<width;i++)
			{
			project[i]=0;
			}
		for (int i=0;i<width;i++)
			{
			for (int j=0;j<index_line;j++)
				{
				project[i]=project[i]+img_t1.at<uchar>(j, i);
				}
			}
	vector <space> v_space;
	int max_index=0;
	g_max=0;
	for (int i=0;i<width;i++)
		{
			if (g_max<project[i])
				{
				g_max=project[i];
				}
			} 
	//	printf("index_line=%d ,gmax=%d \n",index_line,g_max);

	//-------------Calculate the potential space--------------
	int positive[300];
	int num_positive;
	positive[0]=15;//the position of the first space candidate
	num_positive=1; 
	for (int i=15;i<width-15;i++)
		{
		if (project[i]>cw*g_max)
			{
			positive[num_positive]=i;
			num_positive++;
			}
		}
	positive[num_positive]=width-15; //the position of the last space candidate
	num_positive++;
	int enlarge;			

	for (int i=1;i<num_positive;i++)
		{
		space tmp;
		//printf("i=%d positive[i-1] =%d\n",i,positive[i-1]);
		//int width_thres;
		if  ((positive[i-1]>width*0.5)&&(positive[i-1]<width*0.7)) // the position of the character in the upper region, we use bigger threshold for character (it can not be number '1')
			{
			enlarge=5;
			if ((positive[i]-positive[i-1])>36)
				{
				//printf("i=%d positive[i-1] =%d,positive[i]=%d\n",i,positive[i-1],positive[i]);
					if ((positive[i-1]-enlarge)>0)
					{
					tmp.start_point=positive[i-1]-enlarge;
					}
					else
					{
					tmp.start_point=0;
					}
					if ((positive[i]+enlarge)<width-1)
					{
					tmp.end_point=positive[i]+enlarge;
					}
					else
					{
					tmp.end_point=width-1;
					}
				v_space.push_back(tmp);
			}
				else if (positive[i]-positive[i-1]>3)
				{
				positive[i]=positive[i-1];
				}
			}

		else 
			{
			if ((positive[i]-positive[i-1])>3)
				{
		//		printf("i=%d positive[i-1] =%d,positive[i]=%d\n",i,positive[i-1],positive[i]);

					enlarge=5;
				if ((positive[i-1]-enlarge)>12)
					{
					tmp.start_point=positive[i-1]-enlarge;
					}
					else
					{
					tmp.start_point=12;
					}
					if ((positive[i]+enlarge)<width-12)
					{
					tmp.end_point=positive[i]+enlarge;
					}
					else
					{
					tmp.end_point=width-12;
					}
				v_space.push_back(tmp);
			}
			}
		}

	int start_point,end_point;// position of the character
	for (int i=0;i<v_space.size();i++)
		{
		//printf("number of character is %d\n",v_space.size());
		int num_region=0;
		if ((v_space[i].end_point-v_space[i].start_point)<60)
			{
			num_region=1;
			}
		else if ((v_space[i].end_point-v_space[i].start_point)<100)
			{
			num_region=2;
			}
		else if((v_space[i].end_point-v_space[i].start_point)<145)
			{
			num_region=3;
			}
		else if((v_space[i].end_point-v_space[i].start_point)<186)
			{
			num_region=4;
			}
		else
			num_region=5;
		if (num_region==1) // xu li cho 1 region
			{
			if (check_character(img_bw,v_space[i].start_point,v_space[i].end_point,0,index_line)==1)
				{
				start_point=(int) (ratio_width*v_space[i].start_point);
				end_point=(int) (ratio_width*v_space[i].end_point);
				Rect Rec(start_point, 0, end_point-start_point,index_line_ori);
				charRegions.push_back(Rec);
				}
			}
		else 
			{
			//printf("num_region=%d\n",num_region);
			int cut_point[6];
			cut_point[0]=v_space[i].start_point;
			cut_point[num_region]=v_space[i].end_point;
			for (int j=1;j<num_region;j++)
				{
				int mid_point=v_space[i].start_point+j*floor((v_space[i].end_point-v_space[i].start_point)/num_region);
				max=0;
				for (int k=mid_point-10;k<mid_point+10;k++)
					{
					if (max<project[k])
						{
							max=project[k];
							cut_point[j]=k;
						}
					}
				}
			for (int j=0;j<num_region;j++)
				{
			//	printf("region %d\n",j);
				if (check_character(img_bw,cut_point[j],cut_point[j+1],0,index_line)==1)
					{
				//	printf("start=%d, end=%d \n",cut_point[j],cut_point[j+1]);
					start_point=(int) (ratio_width*cut_point[j]);
					end_point=(int) (ratio_width*cut_point[j+1]);
					Rect Rec(start_point, 0, end_point-start_point,index_line_ori);
					charRegions.push_back(Rec);
					}
				}
			}
		}
	v_space.clear();
	//printf("2\n");
	Rect Rec_line(0,0,0,0);
	charRegions.push_back(Rec_line);
	//---------------------------crop character of the lower regions---------------------------------------------------------------------------------
	for (int i=0;i<width;i++)
			{
			project[i]=0;
			}
	for (int i=0;i<width;i++)
		{
		for (int j=index_line;j<height;j++)
			{
			project[i]=project[i]+img_t1.at<uchar>(j, i);
			}
		}
	g_max=0;
	for (int i=0;i<width;i++)
		{
		if (g_max<project[i])
			{
			g_max=project[i];
			}
		} 
	positive[0]=3;//the position of the first space candidate
	num_positive=1; 
	for (int i=1;i<width-1;i++)
		{
		if (project[i]>cw*g_max)
			{
			positive[num_positive]=i;
			num_positive++;
			}
		}
	positive[num_positive]=width-3; //the position of the last space candidate
	num_positive++;
	for (int i=1;i<num_positive;i++)
		{
		space tmp;
		if ((positive[i-1]<6)||(positive[i]>width-6))
			{
			if ((positive[i]-positive[i-1])>22)
				{
					if ((positive[i-1]-enlarge)>0)
					{
					tmp.start_point=positive[i-1]-enlarge;
					}
					else
					{
					tmp.start_point=0;
					}
					if ((positive[i]+enlarge)<width-1)
					{
					tmp.end_point=positive[i]+enlarge;
					}
					else
					{
					tmp.end_point=width-1;
					}
				v_space.push_back(tmp);
			}	
			}
		else
			{
			if ((positive[i]-positive[i-1])>12)
				{
					if ((positive[i-1]-enlarge)>0)
					{
					tmp.start_point=positive[i-1]-enlarge;
					}
					else
					{
					tmp.start_point=0;
					}
					if ((positive[i]+enlarge)<width-1)
					{
					tmp.end_point=positive[i]+enlarge;
					}
					else
					{
					tmp.end_point=width-1;
					}
				v_space.push_back(tmp);
			}
			}	
		}
	for (int i=0;i<v_space.size();i++)
		{
		//printf("v_space.size()=%d\n",v_space.size());
		int num_region=0;
		if ((v_space[i].end_point-v_space[i].start_point)<65)
			{
			num_region=1;
			}
		else if ((v_space[i].end_point-v_space[i].start_point)<100)
			{
			num_region=2;
			}
		else if((v_space[i].end_point-v_space[i].start_point)<145)
			{
			num_region=3;
			}
		else if((v_space[i].end_point-v_space[i].start_point)<186)
			{
			num_region=4;
			}
		else
			num_region=5;
		if (num_region==1) // xu li cho 1 region
			{
			if (check_character(img_bw,v_space[i].start_point,v_space[i].end_point,index_line,height)==1)
				{
				start_point=(int) (ratio_width*v_space[i].start_point);
				end_point=(int) (ratio_width*v_space[i].end_point);
				Rect Rec(start_point, index_line_ori, end_point-start_point,height_ori - index_line_ori - 1);
				//printf("ratio_width=%f, start_point=%d, end_point=%d,index_line_ori=%d\n",ratio_width,start_point,end_point,index_line_ori);
				charRegions.push_back(Rec);
				}
			}
		else 
			{
		//	printf("num_region=%d\n",num_region);
			int cut_point[6];
			cut_point[0]=v_space[i].start_point;
			cut_point[num_region]=v_space[i].end_point;
			for (int j=1;j<num_region;j++)
				{
				int mid_point=v_space[i].start_point+j*floor((v_space[i].end_point-v_space[i].start_point)/num_region);
				max=0;
				for (int k=mid_point-10;k<mid_point+10;k++)
					{
					if (max<project[k])
						{
							max=project[k];
							cut_point[j]=k;
						}
					}
				}
			for (int j=0;j<num_region;j++)
				{
				//printf("region %d\n",j);
				if (check_character(img_bw,cut_point[j],cut_point[j+1],index_line,height)==1)
					{
					//printf("start=%d, end=%d \n",cut_point[j],cut_point[j+1]);
					start_point=(int) (ratio_width*cut_point[j]);
					end_point=(int) (ratio_width*cut_point[j+1]);
					Rect Rec(start_point, index_line_ori, end_point-start_point,height_ori - index_line_ori - 1);
					charRegions.push_back(Rec);
					}
				}
			}
		}
	v_space.clear();

	return charRegions;
}


