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
#include "crop_plate_corner.h"



int region_extend=15;
int plate_extend=10;
int angle_threshold=7;
float dis_extend=10; // number of pixel to extend the plate (after plate corner cropping)
using namespace cv;
using namespace std;

struct line_2point
{
Vec4i lines;  //vector that stores the cordinate of two end points of the line
float angle; // angle of the the line to the horizontal axis
};
struct plate_point
{
Point cn1;
Point cn2;
Point cn3;
Point cn4;
};

Point intersection(Point p1,Point p2,Point p3, Point p4) // the intersection of two lines, 12 and 34
{
Point pt;
if (((p1.x-p2.x)*(p3.y-p4.y)-(p1.y-p2.y)*(p3.x-p4.x))!=0)
	{
	pt.x=((p1.x*p2.y-p1.y*p2.x)*(p3.x-p4.x)-(p1.x-p2.x)*(p3.x*p4.y-p3.y*p4.x))/((p1.x-p2.x)*(p3.y-p4.y)-(p1.y-p2.y)*(p3.x-p4.x));
	pt.y=((p1.x*p2.y-p1.y*p2.x)*(p3.y-p4.y)-(p1.y-p2.y)*(p3.x*p4.y-p3.y*p4.x))/((p1.x-p2.x)*(p3.y-p4.y)-(p1.y-p2.y)*(p3.x-p4.x));
	}
else
	{
	pt.x=0;
	pt.y=0;
	}

return (pt);
}
Mat fill_holes(Mat image)
{
	//cv::Mat image = cv::imread("image.jpg", 0);

	cv::Mat image_thresh;
	cv::threshold(image, image_thresh, 125, 255, cv::THRESH_BINARY);

	// Loop through the border pixels and if they're black, floodFill from there
	cv::Mat mask;
	image_thresh.copyTo(mask);
	for (int i = 0; i < mask.cols; i++) {
	    if (mask.at<char>(0, i) == 0) {
	        cv::floodFill(mask, cv::Point(i, 0), 255, 0, 10, 10);
	    }
	    if (mask.at<char>(mask.rows-1, i) == 0) {
	        cv::floodFill(mask, cv::Point(i, mask.rows-1), 255, 0, 10, 10);
	    }
	}
	for (int i = 0; i < mask.rows; i++) {
	    if (mask.at<char>(i, 0) == 0) {
	        cv::floodFill(mask, cv::Point(0, i), 255, 0, 10, 10);
	    }
	    if (mask.at<char>(i, mask.cols-1) == 0) {
	        cv::floodFill(mask, cv::Point(mask.cols-1, i), 255, 0, 10, 10);
	    }
	}


	// Compare mask with original.
	cv::Mat newImage;
	image.copyTo(newImage);
	for (int row = 0; row < mask.rows; ++row) {
	    for (int col = 0; col < mask.cols; ++col) {
	        if (mask.at<char>(row, col) == 0) {
	            newImage.at<char>(row, col) = 255;
	        }
	    }
	}
	return newImage;
}  
float angle_horizontal(int p1x,int p1y, int p2x, int p2y)
{
double x1 = (float)(p1x - p2x);
double y1 = (float)(p1y - p2y);
double angle;
if (x1 != 0)
angle = atan(y1/x1);
else
angle = 3.14159 / 2.0; // 90 degrees
angle = angle * 180.0 / 3.14159;
//printf("angle=%f \n",angle);
return angle;
}
bool check_plate (Mat img)
{
bool check;// check =1=>plate; check =0=> not plate
check =1;
int width;
int height;
width=img.size().width;
height=img.size().height;
//printf("width=%d, height=%d \n",width,height);
//display_image(img);
Mat img_bin;
Mat img_bin2,img_canny;
bitwise_not(img,img);
adaptiveThreshold(img, img_bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 21,-5);
img_bin2=img_bin;
bitwise_not(img_bin,img_bin);
//display_image(img_canny);
//display_image(img_bin);
int mid_index;
vector<Vec4i> lines;
//-------------Check Horizontal Direction---------------
int hist_horizontal[300];
for (int i=(floor(height/2)-7);i<(floor(height/2)+7);i++)
	{
	hist_horizontal[i]=0;
	}
int max=0;
mid_index=0;
for (int i=(floor(height/2)-7);i<(floor(height/2)+7);i++)
	{
	for (int j=0;j<width;j++)
		{
		if (img_bin.at<uchar>(i, j)>200)
			{
			hist_horizontal[i]=hist_horizontal[i]+1;
			}
		}
	if (max<hist_horizontal[i])
		{
		max=hist_horizontal[i];
		mid_index=i;
		}
	}
if (max<width*0.84)
	{
	check=0;
	}
int hist_vertical_up[300];
int hist_vertical_down[300];

if (check==1)// check vertical direction
	{
//	printf("mid_index=%d\n",mid_index);
	for (int i=0;i<width;i++)
		{
		hist_vertical_up[i]=0;
		hist_vertical_down[i]=0;
		}
	for (int i=0;i<width;i++)
		{
		for (int j=0;j<mid_index;j++)
			{
			if (img_bin.at<uchar>(j, i)>200)
				{
				hist_vertical_up[i]=hist_vertical_up[i]+1;
				}
			}
		for (int j=mid_index;j<height;j++)
			{
			if (img_bin.at<uchar>(j, i)>200)
				{
				hist_vertical_down[i]=hist_vertical_down[i]+1;
				}

			}
		}
	int count=0;
	for (int i=0;i<width;i++)
		{
		if (hist_vertical_up[i]>mid_index*0.9)
			{
			hist_vertical_up[i]=1;
			count++;
			}
		}
	//printf("count_up=%d \n",count);

	if (count<10)
	{check=0;}
	count=0;
	for (int i=0;i<width;i++)
		{
		if (hist_vertical_down[i]>(height-mid_index)*0.9)
			{
			hist_vertical_down[i]=1;
			count++;
			}
		}
	//printf("count_down=%d \n",count);
	if (count<10)
	{check=0;}
}
//-------Check vertical line-------------
/*HoughLinesP(img_canny, lines, 1, CV_PI/180, height/4, height/4, height/6);
double angle_v = 0;
unsigned nb_lines_v = 0;
double angle_h = 0;
unsigned nb_lines_h = 0;
Mat cdst;
cdst=img;
cvtColor(cdst, cdst, CV_GRAY2BGR);
for (int i=0;i<lines.size();i++)
	{
	line( cdst, Point(lines[i][0], lines[i][1]), Point(lines[i][2], lines[i][3]), Scalar(0,0,255), 1, CV_AA);
	float angle_tmp=angle_horizontal(lines[i][0],lines[i][1],lines[i][2],lines[i][3]);
	if ((angle_tmp>50)||(angle_tmp<-50))
		{
		angle_h=angle_h+abs(angle_tmp);
		nb_lines_h++;
		}
	else if ((angle_tmp<20)&&(angle_tmp>-20))
		{
		angle_v=angle_v+abs(angle_tmp);
		nb_lines_v++;
		}
	}
if (nb_lines_h!=0)
	{
	angle_h=angle_h/nb_lines_h;
	}
else
	{
	angle_h=0;
	}
if (nb_lines_v!=0)
	{
	angle_v=angle_v/nb_lines_v;
	}
else
	{
	angle_v=0;
	}
if (angle_h<82)
	{
	check=0;
	}*/
//printf("max=%d width=%d check=%d \n",max,width,check);
//display_image(img_bin2);
//display_image(cdst);

return check;
}
Mat thresholding (Mat img)
{
Mat img_t1; //image after thresholding, hole filling, closing,...
int type;
type = img.type();
adaptiveThreshold(img, img_t1, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 5,0);
//display_image(img_t1);
	int n=img.size().height;
	int m=img.size().width;	
	int dx[4]={-1,0,1,0};
	int dy[4]={0,1,0,-1};
	queue <Point> myqueue;
	int a[200][200],check[200][200];
	for(int j=0;j<n;j++)
	    	for (int k=0;k<m;k++)
	    	{
	    		a[j][k]=(int)img_t1.at<uchar>(j,k);
	    		check[j][k]=1;
	        }
			 Point p,q;
			 p.x = n/2;
			 p.y = m/2;
			 int eps_range=5;
			for (int j=p.x-eps_range;j<=p.x+eps_range;j++)
			{
				for (int k=p.y-eps_range;k<=p.y+eps_range;k++)
				{
					if (a[j][k]==255)
					{
					q.x=j;q.y=k;
					check[j][k]=0;
					myqueue.push(q);
					}
				}
			}

			while (!myqueue.empty())
			{
			q=myqueue.front();
			myqueue.pop();
			for (int j=0;j<4;j++)
			{
				Point td;
				td.x = q.x+dx[j];
				td.y = q.y+dy[j];
				if ((check[td.x][td.y]==1) && (a[td.x][td.y]==255))
				{
					myqueue.push(td);
					check[td.x][td.y]=0;
				}
			}
			}
for (int j=0;j<n;j++)
		{
			for (int k=0;k<m;k++)
			{
				if (check[j][k]==0)
					img_t1.at<uchar>(j,k) = 255;
				else img_t1.at<uchar>(j,k) = 0;}
		}
		img_t1=fill_holes(img_t1);

		Mat const structure_elem = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2));
		morphologyEx(img_t1, img_t1,cv::MORPH_CLOSE, structure_elem);
		img_t1=fill_holes(img_t1); 

return (img_t1);
}

float check_score(Vec2f can_vl,Vec2f can_vr,Vec2f can_hu,Vec2f can_hd,Mat img)
{
Point pt11,pt12,pt21,pt22,pt31,pt32,pt41,pt42; //the cordinate of 4 lines_end points
double a,b,x0,y0;
float v_distance=0; // the distance of angle between 2 vertical lines
float h_distance=0; // the distance of angle between 2 horizontal lines
float ratio_distance=0;// the distance in ration (in compare with the standard plate)
float angle_vl=can_vl[1];
float angle_vr=can_vr[1];
float angle_hd=can_hd[1];
float angle_hu=can_hu[1];
float rho_vl,rho_vr,rho_hu,rho_hd;
rho_vl=can_vl[0];
rho_vr=can_vr[0];
rho_hd=can_hd[0];
rho_hu=can_hu[0];
//------determine the candidate of line_end_point
a = cos(angle_vl); b = sin(angle_vl);
x0 = a*rho_vl; y0 = b*rho_vl;
pt11.x = cvRound(x0 + 1000*(-b));
pt11.y = cvRound(y0 + 1000*(a));
pt12.x = cvRound(x0 - 1000*(-b));
pt12.y = cvRound(y0 - 1000*(a));

a = cos(angle_vr); b = sin(angle_vr);
x0 = a*rho_vr; y0 = b*rho_vr;
pt21.x = cvRound(x0 + 1000*(-b));
pt21.y = cvRound(y0 + 1000*(a));
pt22.x = cvRound(x0 - 1000*(-b));
pt22.y = cvRound(y0 - 1000*(a));	

a = cos(angle_hu); b = sin(angle_hu);
x0 = a*rho_hu; y0 = b*rho_hu;
pt31.x = cvRound(x0 + 1000*(-b));
pt31.y = cvRound(y0 + 1000*(a));
pt32.x = cvRound(x0 - 1000*(-b));
pt32.y = cvRound(y0 - 1000*(a));

a = cos(angle_hd); b = sin(angle_hd);
x0 = a*rho_hd; y0 = b*rho_hd;
pt41.x = cvRound(x0 + 1000*(-b));
pt41.y = cvRound(y0 + 1000*(a));
pt42.x = cvRound(x0 - 1000*(-b));
pt42.y = cvRound(y0 - 1000*(a));
//----------------------------------------------
Point cn1,cn2,cn3,cn4;
cn3=intersection(pt31,pt32,pt21,pt22);
cn4=intersection(pt41,pt42,pt21,pt22);
cn1=intersection(pt11,pt12,pt31,pt32);
cn2=intersection(pt11,pt12,pt41,pt42);
int height,width;
height=img.size().height;
width=img.size().width;
Mat img_affine;
Point2f a2(0, 0), b2(width, 0), c2(width, height),d2(0,height);
Point2f src_point[] = {cn1, cn2,cn3,cn4};
Point2f dst_point[] = {a2, d2, b2,c2};
Mat warpMat = getAffineTransform(src_point, dst_point);
warpAffine(img,img_affine, warpMat, img.size());
bool check;
check=check_plate(img_affine);
angle_vr=angle_vr* 180.0 / 3.14159;
angle_vl=angle_vl* 180.0 / 3.14159;
angle_hd=angle_hd* 180.0 / 3.14159;
angle_hu=angle_hu* 180.0 / 3.14159;
//----------------------------------------------
if ((angle_vr!=0)||(angle_vl!=0))
	{
	v_distance=abs(abs(angle_vl)-abs(angle_vr))/angle_threshold;
	}
else
	{
	v_distance=0;
	}
if ((angle_hu!=0)||(angle_hd!=0))
	{
	h_distance=abs(angle_hu-angle_hd)/angle_threshold;
	}
else
	{
	h_distance=0;
	}
float ratio_car=1.4;
float ratio_bike=1.357;
float ratio_long=4.27;
float d_vl=norm(cn1-cn2);//length of vertical left line
float d_vr=norm(cn3-cn4);//length of vertical right line
float d_hu=norm(cn1-cn3);//length of horizontal up line
float d_hd=norm(cn2-cn4);//length of horizontal down line
float ratio=(d_hu+d_hd)/(d_vl+d_vr);
int height_approx,width_approx;
height_approx=height-2*region_extend;
width_approx=width-2*region_extend;
//ratio_distance=abs(ratio-ratio_long)/ratio_long;
ratio_distance=abs(ratio-ratio_car)/ratio_car;
float total_distance;
total_distance=9999;
if ((d_vl<1.2*height_approx)&&(d_vl>0.5*height_approx)&&(d_hu<1.2*width_approx)&&(d_hu>0.5*width_approx)&&(d_hd<1.2*width_approx)&&(d_hd>0.5*width_approx)&&(d_vr<1.2*height_approx)&&(d_vr>0.5*height_approx))
	{
	if ((v_distance<1)&&(h_distance<1)&&(ratio<1.7)&&(ratio>1.2&&(check==1)))
	{
	total_distance=v_distance+h_distance+ratio_distance;
	}
else
	{
	total_distance=9999;
	}
	}
//printf("v=%f  h=%f  r=%f final =%f\n",v_distance,h_distance,ratio_distance,total_distance);
return(total_distance);
}
float croping_v2(Mat img_src,Point &cn1, Point &cn2, Point &cn3, Point &cn4,float ratio)
{
Mat img;
img=thresholding(img_src);
Mat img_canny;
Canny(img,img_canny,50,150,3);
//display_image(img_canny);
int height=img.size().height;
int width=img.size().width;
 vector<Vec2f> lines; //vector that stores all the lines detected by Hough Transform
 vector<Vec2f> lines_hu; // vector that stores all the candidates for the upper horizontal line 
 vector<Vec2f> lines_hd;// vector that stores all the candidates for the lower horizontal line
 vector<Vec2f> lines_vl;// vector that stores all the candidates for the left vertical line
 vector<Vec2f> lines_vr;// vector that stores all the candidates for the right vertical line 
 int thres_vote=floor(height/4);// minimum votes for a line (in Hough transform)
 HoughLines(img_canny,lines, 1, CV_PI/180,thres_vote, 0, 0 ); // Hough transform to detect lines whose votes> thres_vote. All lines are saved into vector lines
 //printf("thres_vote=%d,number of lines is %d\n",thres_vote,lines.size());
 Point pt1,pt2,pt3,pt4,pt5,pt6; //pt3,pt4,pt5,pt6 are 4 points at conrner of the image
 pt3.x=0; pt3.y=0; //top left corner;
 pt4.x=0; pt4.y=height-1;// bottom left corner;
 pt5.x=width-1;pt5.y=0;//top right cornenr
 pt6.x=width-1;pt6.y=height-1;//bottom right corner
 float interthres=0.5; //threshold to decide if a lines belong to hu,hd,vl,vr or not 
 //------------Detect line candidates and store them in the correspond vector-------------
 for (int i=0;i<lines.size();i++)
	{
	float rho,theta;
	double a,b,x0,y0; 
	Point int1,int2;
	rho = lines[i][0];
	theta = lines[i][1];
	a = cos(theta);
	b = sin(theta);
	x0 = a*rho;
	y0 = b*rho;
	pt1.x = cvRound(x0 + 1000*(-b));
    pt1.y = cvRound(y0 + 1000*(a));
    pt2.x = cvRound(x0 - 1000*(-b));
    pt2.y = cvRound(y0 - 1000*(a));
	if ( theta>CV_PI/180*155 || theta<CV_PI/180*25)
	 {
	interthres=0.5;
	 int1=intersection(pt1,pt2,pt3,pt5);
	 int2=intersection(pt1,pt2,pt4,pt5);
	 if ((int1.x<interthres*width)&&(int2.x<interthres*width))
		{
			lines_vl.push_back(lines[i]);
		}
	 else if ((int1.x>(1-interthres)*width)&&(int2.x>(1-interthres)*width))
		{
			lines_vr.push_back(lines[i]);
		}
	 }
	else if ( theta>CV_PI/180*65 && theta<CV_PI/180*115)
		{
			interthres=0.5;
		int1=intersection(pt1,pt2,pt3,pt4);
		int2=intersection(pt1,pt2,pt5,pt6);
	if ((int1.y<interthres*height)&&(int2.y<interthres*height))
		{
			lines_hu.push_back(lines[i]);
		}
	 else if ((int1.y>(1-interthres)*height)&&(int2.y>(1-interthres)*height))
		{
			lines_hd.push_back(lines[i]);
		}

		}
	}
//printf("vl=%d, vr=%d, hu=%d, hu=%d\n",lines_vl.size(),lines_vr.size(),lines_hd.size(),lines_hu.size());
 //----------------- End of line detection------------------------------------------
 //------------------Preparation to find the best set of lines to form the license plate-----------
int max_line=3; // We check "max_line" number of candidate from each vector
int num_vl=0; // The actual number of vertical left to be checked, this could be different from max_line if lines_vl.size()<max_line
int num_vr=0;
int num_hu=0;
int num_hd=0;
int num_lines=0;//Number of lines that can be detected 
float rho,theta;
double a,b,x0,y0;
Point pt11,pt12,pt21,pt22,pt31,pt32,pt41,pt42; //the cordinate of 4 lines_end points
int type=0; //type =1:miss vl, type =2: miss vr, type =3: miss hu, type =4: miss hd
max_line=4;
if (lines_vl.size()>0)
   {
	if (lines_vl.size()>max_line)
		{
		num_vl=max_line;
		}
	else
		{
		num_vl=lines_vl.size();
		}
	rho = lines_vl[0][0]; theta = lines_vl[0][1];
	a = cos(theta); b = sin(theta);
	x0 = a*rho; y0 = b*rho;
    pt11.x = cvRound(x0 + 1000*(-b));
    pt11.y = cvRound(y0 + 1000*(a));
    pt12.x = cvRound(x0 - 1000*(-b));
    pt12.y = cvRound(y0 - 1000*(a));
	num_lines++;
   }
else
	{
	pt11=pt3;pt12=pt4;
	type=1;
	}
if (lines_vr.size()>0)
   {
	if (lines_vr.size()>max_line)
		{
		num_vr=max_line;
		}
	else
		{
		num_vr=lines_vr.size();
		}
	rho = lines_vr[0][0]; theta = lines_vr[0][1];
	a = cos(theta); b = sin(theta);
	x0 = a*rho; y0 = b*rho;
    pt21.x = cvRound(x0 + 1000*(-b));
    pt21.y = cvRound(y0 + 1000*(a));
    pt22.x = cvRound(x0 - 1000*(-b));
    pt22.y = cvRound(y0 - 1000*(a));
	num_lines++;
   }
else
	{
	pt21=pt5;pt22=pt6;
	type=2;
	}
max_line=2;
if (lines_hu.size()>0)
   {
	if (lines_hu.size()>max_line)
		{
		num_hu=max_line;
		}
	else
		{
		num_hu=lines_hu.size();
		}
	rho = lines_hu[0][0]; theta = lines_hu[0][1];
	a = cos(theta); b = sin(theta);
	x0 = a*rho; y0 = b*rho;
    pt31.x = cvRound(x0 + 1000*(-b));
    pt31.y = cvRound(y0 + 1000*(a));
    pt32.x = cvRound(x0 - 1000*(-b));
    pt32.y = cvRound(y0 - 1000*(a));
	num_lines++;
   }
else
	{
	pt31=pt3;pt32=pt5;
	type=3;
	}
if (lines_hd.size()>0)
   {
	if (lines_hd.size()>max_line)
		{
		num_hd=max_line;
		}
	else
		{
		num_hd=lines_hd.size();
		}
	rho = lines_hd[0][0]; theta = lines_hd[0][1];
	a = cos(theta); b = sin(theta);
	x0 = a*rho; y0 = b*rho;
    pt41.x = cvRound(x0 + 1000*(-b));
    pt41.y = cvRound(y0 + 1000*(a));
    pt42.x = cvRound(x0 - 1000*(-b));
    pt42.y = cvRound(y0 - 1000*(a));
	num_lines++;
   }
else
	{
	pt41=pt4;pt42=pt6;
	type=4;
	}
//printf("vl=%d vr=%d hu=%d hd=%d\n",num_vl,num_vr,num_hd,num_hu);
//--------------Start to find the best set of candidate to form the plate-------------------------------
int index_vl,index_vr,index_hu,index_hd; //index of chosen line
float plate_score;
Vec2f can_vl,can_vr,can_hu,can_hd; // the candidate lines to be check
float d1,d2,delta_x,delta_y;
//printf("num_lines=%d\n",num_lines);
if (num_lines==4)
	{
	plate_score=99999; //the score that determine how likely the candidates could form a plate. The smaller score is, the more confident we have
	for (int i=0;i<num_vl;i++)
		{
		for (int j=0;j<num_vr;j++)
			{
			for (int k=0;k<num_hu;k++)
				{
				for (int h=0;h<num_hd;h++)
					{
					can_vl=lines_vl[i];
					can_vr=lines_vr[j];
					can_hu=lines_hu[k];
					can_hd=lines_hd[h];
					float score_tmp=check_score(can_vl,can_vr,can_hu,can_hd,img_src);
					//printf("score=%f\n",score_tmp);
					if (plate_score>score_tmp)
						{
						plate_score=score_tmp;
						index_vl=i;
						index_vr=j;
						index_hu=k;
						index_hd=h;
						}
					}
				}
			}
		}
	
can_vl=lines_vl[index_vl];
can_vr=lines_vr[index_vr];
can_hu=lines_hu[index_hu];
can_hd=lines_hd[index_hd];
//printf("index,vl=%d,vr=%d,hu=%d,hd=%d\n",index_vl,index_vr,index_hu,index_hd);
float angle_vl=can_vl[1];
float angle_vr=can_vr[1];
float angle_hd=can_hd[1];
float angle_hu=can_hu[1];
float rho_vl,rho_vr,rho_hu,rho_hd;
rho_vl=can_vl[0];
rho_vr=can_vr[0];
rho_hd=can_hd[0];
rho_hu=can_hu[0];
//------determine the candidate of line_end_point
a = cos(angle_vl); b = sin(angle_vl);
x0 = a*rho_vl; y0 = b*rho_vl;
pt11.x = cvRound(x0 + 1000*(-b));
pt11.y = cvRound(y0 + 1000*(a));
pt12.x = cvRound(x0 - 1000*(-b));
pt12.y = cvRound(y0 - 1000*(a));

a = cos(angle_vr); b = sin(angle_vr);
x0 = a*rho_vr; y0 = b*rho_vr;
pt21.x = cvRound(x0 + 1000*(-b));
pt21.y = cvRound(y0 + 1000*(a));
pt22.x = cvRound(x0 - 1000*(-b));
pt22.y = cvRound(y0 - 1000*(a));	

a = cos(angle_hu); b = sin(angle_hu);
x0 = a*rho_hu; y0 = b*rho_hu;
pt31.x = cvRound(x0 + 1000*(-b));
pt31.y = cvRound(y0 + 1000*(a));
pt32.x = cvRound(x0 - 1000*(-b));
pt32.y = cvRound(y0 - 1000*(a));

a = cos(angle_hd); b = sin(angle_hd);
x0 = a*rho_hd; y0 = b*rho_hd;
pt41.x = cvRound(x0 + 1000*(-b));
pt41.y = cvRound(y0 + 1000*(a));
pt42.x = cvRound(x0 - 1000*(-b));
pt42.y = cvRound(y0 - 1000*(a));
//----------------------------------------------
cn3=intersection(pt31,pt32,pt21,pt22);
cn4=intersection(pt41,pt42,pt21,pt22);
cn1=intersection(pt11,pt12,pt31,pt32);
cn2=intersection(pt11,pt12,pt41,pt42);
}
/*
else if (num_lines==3)
	{
	if (type==1)
		{
		cn3=intersection(pt31,pt32,pt21,pt22);
		cn4=intersection(pt41,pt42,pt21,pt22);
		d1=norm(cn3-cn4); //distance between cn3, cn4
		d2=d1*ratio;
		delta_x=d2/sqrt(1+pow((cn3.y-pt32.y)/(cn3.x-pt32.x),2));
		delta_y=delta_x*(cn3.y-pt32.y)/(cn3.x-pt32.x);
		cn1.x=cn3.x-delta_x;
		cn1.y=cn3.y-delta_y;
		cn2.x=cn4.x-delta_x;
		cn2.y=cn4.y-delta_y;
		}
	else if (type==2)
		{
		cn1=intersection(pt11,pt12,pt31,pt32);
		cn2=intersection(pt11,pt12,pt41,pt42);
		d1=norm(cn1-cn2);
		d2=d1*ratio;
		delta_x=d2/sqrt(1+pow((cn1.y-pt32.y)/(cn1.x-pt32.x),2));
		delta_y=delta_x*(cn1.y-pt32.y)/(cn1.x-pt32.x);
		cn3.x=cn1.x+delta_x;
		cn3.y=cn1.y+delta_y;
		cn4.x=cn2.x+delta_x;
		cn4.y=cn2.y+delta_y;
		}
	else if (type==3)
		{
		cn4=intersection(pt21,pt22,pt41,pt42);
		cn2=intersection(pt11,pt12,pt41,pt42);
		d1=norm(cn4-cn2);
		d2=d1/ratio;
		delta_y=d2/sqrt(1+pow((pt12.x-cn2.x)/(cn2.y-pt12.y),2));
		delta_x=delta_y*(pt12.x-cn2.x)/(pt12.y-cn2.y);
		cn1.x=cn2.x-delta_x;
		cn1.y=cn2.y-delta_y;
		cn3.x=cn4.x-delta_x;
		cn3.y=cn4.y-delta_y;
		}
	else if (type==4)
		{
		cn1=intersection(pt11,pt12,pt31,pt32);
		cn3=intersection(pt31,pt32,pt21,pt22);
		d1=norm(cn1-cn3);
		d2=d1/ratio;
		delta_y=d2/sqrt(1+pow((pt12.x-cn1.x)/(cn1.y-pt12.y),2));
		delta_x=delta_y*(pt12.x-cn1.x)/(pt12.y-cn1.y);
		cn2.x=cn1.x+delta_x;
		cn2.y=cn1.y+delta_y;
		cn4.x=cn3.x+delta_x;
		cn4.y=cn3.y+delta_y;
		}
	plate_score=100;
	}
	*/
else
	{
	plate_score=9999;
	}
return(plate_score);
}
int intersection_vertical(line_2point l)  // find the intersection of a line to the vertical axis
{
int intersection;
Point p1,p2,p3,p4;
p1.x=l.lines[0];
p1.y=l.lines[1];
p2.x=l.lines[2];
p2.y=l.lines[3];
p3.x=0;
p3.y=0;
p4.x=0;
p4.y=100;
intersection=((p1.x*p2.y-p1.y*p2.x)*(p3.y-p4.y)-(p1.y-p2.y)*(p3.x*p4.y-p3.y*p4.x))/((p1.x-p2.x)*(p3.y-p4.y)-(p1.y-p2.y)*(p3.x-p4.x));
return (intersection);
}
int intersection_horizontal(line_2point l)  // find the intersection of a line to the horizontal axis
{
int intersection;
Point p1,p2,p3,p4;
p1.x=l.lines[0];
p1.y=l.lines[1];
p2.x=l.lines[2];
p2.y=l.lines[3];
p3.x=0;
p3.y=0;
p4.x=100;
p4.y=0;
intersection=((p1.x*p2.y-p1.y*p2.x)*(p3.x-p4.x)-(p1.x-p2.x)*(p3.x*p4.y-p3.y*p4.x))/((p1.x-p2.x)*(p3.y-p4.y)-(p1.y-p2.y)*(p3.x-p4.x));
return (intersection);
}
float croping_v3(Mat img,Point &corner1, Point &corner2, Point &corner3, Point &corner4)
{
//display_image(img);
//equalizeHist( img, img );
//display_image(img);
Mat img_canny;
Mat img_sobel,grad;
Mat img1=thresholding(img); 
Canny(img1,img_canny,50,150,3);
//-----------Edge Sobel detection--------------------
int scale = 1;
int delta = 0;
int ddepth = CV_16S;
GaussianBlur( img, img, Size(3,3), 0, 0, BORDER_DEFAULT );
Mat grad_x, grad_y;
Mat abs_grad_x, abs_grad_y;
Sobel( img, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT );
convertScaleAbs( grad_x, abs_grad_x );
Sobel( img, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT );
convertScaleAbs( grad_y, abs_grad_y );
addWeighted( abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad );
//display_image(grad);

threshold( grad, img_sobel, 90,255,THRESH_BINARY );

int height=img.size().height;
int width=img.size().width;
//-------------Lines detection with hough transform-------------------
vector<Vec4i> lines_canny;
vector<Vec4i>lines_sobel;
vector<line_2point> lines;
vector<line_2point> lines_vl;
vector<line_2point> lines_vr;
vector<line_2point> lines_hu;
vector<line_2point> lines_hd;
Mat dst, cdst;
cvtColor(img, cdst, CV_GRAY2BGR);
int height_approx,width_approx; // the approximate size of the plate (size before region extend)
height_approx=height-2*region_extend;
width_approx=width-2*region_extend;
//printf("height=%d width=%d height_approx=%d, width_approx=%d\n",height,width,height_approx,width_approx);
HoughLinesP(img_sobel, lines_sobel, 1, CV_PI/180, height_approx/3, height_approx/3, height_approx);// line detected by sobel edge detection
HoughLinesP(img_canny, lines_canny, 1, CV_PI/180,height_approx/3, height_approx/3, height_approx); // line detected by canny edge detection
for (int i=0;i<lines_sobel.size();i++)
	{
	line_2point tmp;
	Vec4i l = lines_sobel[i];
	tmp.lines=lines_sobel[i];
	tmp.angle=angle_horizontal(l[0],l[1],l[2],l[3]);
	lines.push_back(tmp);
	}
for (int i=0;i<lines_canny.size();i++)
	{
	line_2point tmp;
	Vec4i l = lines_canny[i];
	tmp.lines=lines_canny[i];
	tmp.angle=angle_horizontal(l[0],l[1],l[2],l[3]);
	lines.push_back(tmp);
	}
for (int i=0;i<lines.size();i++)
	{
	if ((lines[i].angle<20)&&(lines[i].angle>-20))
		{
		if ((lines[i].lines[1]<height/2)&&(lines[i].lines[3]<height/2))
			{
			//check if there exist duplicated line 
			int check=1;
			int cross=intersection_vertical(lines[i]);
			//printf("hu cross=%d, angle=%f\n",cross,lines[i].angle);
			for (int j=0;j<lines_hu.size();j++)
				{
				if ((abs(abs(lines[i].angle)-abs(lines_hu[j].angle))<2)&&(abs(cross-intersection_vertical(lines_hu[j]))<3))
					{
					check=0;
					break;
					}
				}
			if (check==1)
				{
				lines_hu.push_back(lines[i]);
				line( cdst, Point(lines[i].lines[0], lines[i].lines[1]), Point(lines[i].lines[2], lines[i].lines[3]), Scalar(0,0,255), 1, CV_AA);
				//display_image(cdst);
				}
			}
		else if ((lines[i].lines[1]>height/2)&&(lines[i].lines[3]>height/2))
			{
			//check if there exist duplicated line 
			int check=1;
			int cross=intersection_vertical(lines[i]);
			//printf("hd cross=%d, angle=%f\n",cross,lines[i].angle);
			for (int j=0;j<lines_hd.size();j++)
				{
				if ((abs(abs(lines[i].angle)-abs(lines_hd[j].angle))<2)&&(abs(cross-intersection_vertical(lines_hd[j]))<3))
					{
					check=0;
					break;
					}
				}
			if (check==1)
				{
				lines_hd.push_back(lines[i]);
				line( cdst, Point(lines[i].lines[0], lines[i].lines[1]), Point(lines[i].lines[2], lines[i].lines[3]), Scalar(0,0,255), 1, CV_AA);
				//display_image(cdst);
				}
			}
		}
	if ((lines[i].angle>70)||(lines[i].angle<-70))
		{
		if ((lines[i].lines[0]<width/3)&&(lines[i].lines[2]<width/3))
			{
			//check if there exist duplicated line 
			int check=1;
			int cross=intersection_horizontal(lines[i]);
			//printf("vl cross=%d, angle=%f\n",cross,lines[i].angle);
			for (int j=0;j<lines_vl.size();j++)
				{
				if ((abs(abs(lines[i].angle)-abs(lines_vl[j].angle))<2)&&(abs(cross-intersection_horizontal(lines_vl[j]))<3))
					{
					check=0;
					break;
					}
				}
			if (check==1)
				{
				lines_vl.push_back(lines[i]);
				line( cdst, Point(lines[i].lines[0], lines[i].lines[1]), Point(lines[i].lines[2], lines[i].lines[3]), Scalar(0,0,255), 1, CV_AA);
				//display_image(cdst);
				}
			}
		else if ((lines[i].lines[0]>2*width/3)&&(lines[i].lines[2]>2*width/3))
			{
			//check if there exist duplicated line 
			int check=1;
			int cross=intersection_horizontal(lines[i]);
			//printf("vr cross=%d, angle=%f\n",cross,lines[i].angle);
			for (int j=0;j<lines_vr.size();j++)
				{
				if ((abs(abs(lines[i].angle)-abs(lines_vr[j].angle))<2)&&(abs(cross-intersection_horizontal(lines_vr[j]))<3))
					{
					check=0;
					break;
					}
				}
			if (check==1)
				{
				lines_vr.push_back(lines[i]);
				line( cdst, Point(lines[i].lines[0], lines[i].lines[1]), Point(lines[i].lines[2], lines[i].lines[3]), Scalar(0,0,255), 1, CV_AA);
				//display_image(cdst);
				}
			}
		}
	}
//printf("number of vl=%d, vr=%d, hu=%d, hd=%d \n",lines_vl.size(),lines_vr.size(),lines_hu.size(),lines_hd.size());
int num_lines=0;
int type=0;//type =1:miss vl, type =2: miss vr, type =3: miss hu, type =4: miss hd
//------------erase duplicate lines--------------------------------------
if (lines_vl.size()>0)
	{
	num_lines++;
	}
else 
	{
	type=1;
	}
if (lines_vr.size()>0)
	{
	num_lines++;
	}
else
	{
	type=2;
	}
if (lines_hu.size()>0)
	{
	num_lines++;
	}
else 
	{
	type=3;
	}
if (lines_hd.size()>0)
	{
	num_lines++;
	}
else 
	{
	type=4;
	}
//printf("lines.size=%d \n",lines.size());
//cout<<"type of line detection is "<<type<<endl;
//printf("angle_threshold=%d\n",angle_threshold);

//line( cdst, Point(lines[i].lines[0], lines[i].lines[1]), Point(lines[i].lines[2], lines[i].lines[3]), Scalar(0,0,255), 1, CV_AA);
//display_image(img_sobel);
//display_image(img_canny);
//display_image(cdst);

vector<Vec2i> vertical_pair; //candidate for vertical pairs of lines
vector<Vec2i> horizontal_pair; //candidate for horizontal pairs of lines
for (int i=0;i<lines_vl.size();i++)
	{
	for (int j=0;j<lines_vr.size();j++)
		{
		//printf("angle[vl]=%f, angle[vr]=%f\n",lines_vl[i].angle,lines_vr[j].angle);
		if (abs(abs(lines_vl[i].angle)-abs(lines_vr[j].angle))<angle_threshold)
			{
			Vec2i tmp;
			tmp[0]=i;
			tmp[1]=j;
			vertical_pair.push_back(tmp);
			//cout<<"ver angle1="<<lines_vl[i].angle<<"\t"<<"angle2="<<lines_vr[j].angle<<endl;
			}
		}
	}
for (int i=0;i<lines_hu.size();i++)
	{
	for (int j=0;j<lines_hd.size();j++)
		{
		if (abs(abs(lines_hu[i].angle)-abs(lines_hd[j].angle))<angle_threshold)
			{
			Vec2i tmp;
			tmp[0]=i;
			tmp[1]=j;
			horizontal_pair.push_back(tmp);
			//cout<<" ho angle1="<<lines_hu[i].angle<<"\t"<<"angle2="<<lines_hd[j].angle<<endl;
			}
		}
	}
int count=0;
Mat img_clone;
float min_distance=9999;
plate_point plate_tmp;

//printf("number of horizontal pair=%d, vertical pair=%d \n",horizontal_pair.size(),vertical_pair.size());
for (int i=0;i<vertical_pair.size();i++)
//for (int i=0;i<1;i++)
	{
	for (int j=0;j<horizontal_pair.size();j++)
//for (int j=2;j<3;j++)
		{
		
		Point p11,p12,p21,p22,p31,p32,p41,p42;
		Point cn1,cn2,cn3,cn4;
		p11.x=lines_vl[vertical_pair[i][0]].lines[0];
		p11.y=lines_vl[vertical_pair[i][0]].lines[1];
		p12.x=lines_vl[vertical_pair[i][0]].lines[2];
		p12.y=lines_vl[vertical_pair[i][0]].lines[3];
		p31.x=lines_vr[vertical_pair[i][1]].lines[0];
		p31.y=lines_vr[vertical_pair[i][1]].lines[1];
		p32.x=lines_vr[vertical_pair[i][1]].lines[2];
		p32.y=lines_vr[vertical_pair[i][1]].lines[3];
		p21.x=lines_hd[horizontal_pair[j][1]].lines[0];
		p21.y=lines_hd[horizontal_pair[j][1]].lines[1];
		p22.x=lines_hd[horizontal_pair[j][1]].lines[2];
		p22.y=lines_hd[horizontal_pair[j][1]].lines[3];
		p41.x=lines_hu[horizontal_pair[j][0]].lines[0];
		p41.y=lines_hu[horizontal_pair[j][0]].lines[1];
		p42.x=lines_hu[horizontal_pair[j][0]].lines[2];
		p42.y=lines_hu[horizontal_pair[j][0]].lines[3];
		cn1=intersection(p11,p12,p41,p42);
		cn2=intersection(p11,p12,p21,p22);
		cn3=intersection(p21,p22,p31,p32);
		cn4=intersection(p31,p32,p41,p42);
		float length_vl,length_hu,ratio,length_hd,length_vr;
		float angle_vl,angle_vr,angle_hu,angle_hd;
		length_vl=norm(cn1-cn2);
		length_hu=norm(cn1-cn4);
		length_hd=norm(cn2-cn3);
		length_vr=norm(cn3-cn4);
		float ratio_vertical,ratio_horizontal;
		if (max(length_vl,length_vr)!=0)
			{
			ratio_vertical=min(length_vr,length_vl)/max(length_vr,length_vl);
			}
		else
			{
			ratio_vertical=0;
			}
		if (max(length_hu,length_hd)!=0)
			{
			ratio_horizontal=min(length_hu,length_hd)/max(length_hu,length_hd);
			}
		else
			{
			ratio_horizontal=0;
			}
		if ((length_vl+length_vr)!=0)
			{
			ratio=(length_hu+length_hd)/(length_vl+length_vr);
			}
		else 
			{
			ratio=0;
			}
		//printf("vl=%f, vr=%f,hu=%f,hd=%f,height=%d, width=%d, ratio=%f,ratio_vertical=%f, ratio_hori=%f \n",length_vl,length_vr,length_hu,length_hd,height_approx,width_approx,ratio,ratio_vertical,ratio_horizontal);
		if ((length_vl<1.2*height_approx)&&(length_vl>0.5*height_approx)&&(length_hu<1.2*width_approx)&&(length_hu>0.5*width_approx)&&(length_hd<1.2*width_approx)&&(length_hd>0.5*width_approx)&&(length_vr<1.2*height_approx)&&(length_vr>0.5*height_approx)&&(ratio<1.65)&&(ratio>1.25)&&(ratio_vertical>0.85)&&(ratio_horizontal>0.85))
		//if ((length_vl<(height-plate_extend))&&(length_vl>0.6*height_approx)&&(length_hu<(width-plate_extend))&&(length_hu>0.6*width_approx)&&(length_hd<(width-plate_extend))&&(length_hd>0.6*width_approx)&&(length_vr<(height-plate_extend))&&(length_vr>0.6*height_approx)&&(ratio<1.55)&&(ratio>1.25))
			{
			//img_clone=cdst;
			Mat img_affine;
			/*line( img_clone, cn1, cn2, Scalar(0,0,255), 1, CV_AA);
			line( img_clone, cn2, cn3, Scalar(0,0,255), 1, CV_AA);
			line( img_clone, cn3, cn4, Scalar(0,0,255), 1, CV_AA);
			line( img_clone, cn4, cn1, Scalar(0,0,255), 1, CV_AA);*/
			cv::Point2f a2(0, 0), b2(width, 0), c2(width, height),d2(0,height);
			cv::Point2f src_point[] = {cn1, cn2,cn3,cn4};
			cv::Point2f dst_point[] = {a2, d2, c2,b2};
			cv::Mat warpMat = getAffineTransform(src_point, dst_point);
			cv::warpAffine(img,img_affine, warpMat, img.size());
			//display_image(img_clone);
			//if (check_plate(img_affine))
			if (check_plate(img_affine))			
				{
				angle_vl=lines_vl[vertical_pair[i][0]].angle;
				angle_vr=lines_vr[vertical_pair[i][1]].angle;
				angle_hu=lines_hu[horizontal_pair[j][0]].angle;
				angle_hd=lines_hd[horizontal_pair[j][1]].angle;
				float v_distance=0; // the distance of angle between 2 vertical lines
				float h_distance=0; 
				if ((angle_vr!=0)||(angle_vl!=0))
					{
					v_distance=abs(abs(angle_vl)-abs(angle_vr))/angle_threshold;
					}
				else
					{
					v_distance=0;
					}
				if ((angle_hu!=0)||(angle_hd!=0))
					{
					h_distance=abs(abs(angle_hu)-abs(angle_hd))/angle_threshold;
					}
				else
					{
					h_distance=0;
					}
				float ratio_car=1.4;
				float ratio_bike=1.357;
				float ratio_long=4.27;
				float ratio_distance;
				ratio_distance=abs(ratio-ratio_car)/ratio_car;
				float total_distance;
				total_distance=v_distance+h_distance+ratio_distance;
				//printf("distance_v=%f, distance_h=%f, ratio=%f total_distance=%f\n",v_distance,h_distance,ratio,total_distance);
				if (total_distance<min_distance)
					{
					min_distance=total_distance;
					plate_tmp.cn1=cn1;
					plate_tmp.cn2=cn2;
					plate_tmp.cn3=cn3;
					plate_tmp.cn4=cn4;
					}
				count++;
				}
			}
		}
	}
//printf("count= %d min_distance=%f \n",count,min_distance);
img_clone=cdst;
/*line( img_clone, plate_tmp.cn1, plate_tmp.cn2, Scalar(0,0,255), 1, CV_AA);
line( img_clone, plate_tmp.cn2, plate_tmp.cn3, Scalar(0,0,255), 1, CV_AA);
line( img_clone, plate_tmp.cn3, plate_tmp.cn4, Scalar(0,0,255), 1, CV_AA);
line( img_clone, plate_tmp.cn4, plate_tmp.cn1, Scalar(0,0,255), 1, CV_AA);
display_image(img_clone);*/
corner1=plate_tmp.cn1;
corner2=plate_tmp.cn2;
corner3=plate_tmp.cn4;
corner4=plate_tmp.cn3;
return (min_distance);
}
Point intersection_general(float a1,float b1, float c1, float a2, float b2, float c2)
{
Point pt;
pt.x=(int)((c1*b2-b1*c2)/(b1*a2-a1*b2));
pt.y=(int)((a1*c2-c1*a2)/(b1*a2-a1*b2));
return (pt);
}
void point_extend (Point &cn1, Point &cn2, Point &cn3, Point &cn4,float dis)
{
// The equation off 4 line , in form of ax+by+c=0;
float a1=(float) (cn1.y-cn3.y); // up line
float b1=(float)(cn3.x-cn1.x);
float c1=(float)(cn1.x*cn3.y-cn3.x*cn1.y);
float a2=(float)(cn2.y-cn4.y);// down line
float b2=(float)(cn4.x-cn2.x);
float c2=(float)(cn2.x*cn4.y-cn4.x*cn2.y);
float a3=(float)(cn1.y-cn2.y);// left line
float b3=(float)(cn2.x-cn1.x);
float c3=(float)(cn1.x*cn2.y-cn2.x*cn1.y);
float a4=(float)(cn3.y-cn4.y); // right
float b4=(float)(cn4.x-cn3.x);
float c4=(float)(cn3.x*cn4.y-cn4.x*cn3.y);
float c11=c1+sqrt(a1*a1+b1*b1)*dis;
float c22=c2-sqrt(a2*a2+b2*b2)*dis;
float c33=c3-sqrt(a3*a3+b3*b3)*dis;
float c44=c4+sqrt(a4*a4+b4*b4)*dis;
cn1=intersection_general(a1,b1,c11,a3,b3,c33);
cn2=intersection_general(a2,b2,c22,a3,b3,c33);
cn3=intersection_general(a1,b1,c11,a4,b4,c44);
cn4=intersection_general(a2,b2,c22,a4,b4,c44);
}

plate_corner crop_plate_corner (Mat img_rgb,int mode)
{
Mat img;
cvtColor(img_rgb, img,CV_RGB2GRAY);
plate_corner tmp;
Mat img_src=img;
int width=img.size().width;
int height=img.size().height;
Point cn1, cn2, cn3, cn4;  // To be done :initialize the value of cn1, cn2,cn3, cn4
float result=9999; 
int check;
float ratio=1.4;
result= croping_v2(img,cn1,cn2,cn3,cn4,ratio);
check=0;
if (result<99)
	{
	tmp.cn1=cn1;
	tmp.cn2=cn2;
	tmp.cn3=cn3;
	tmp.cn4=cn4;
	tmp.plate_score=result;
	}
else
	{
	//printf("cropping v3\n");
	result=croping_v3(img,cn1,cn2,cn3,cn4);
	if (result<100)
		{
		tmp.cn1=cn1;
		tmp.cn2=cn2;
		tmp.cn3=cn3;
		tmp.cn4=cn4;
		tmp.plate_score=result;
		}
	else
		{
		tmp.plate_score=9999;
		}

	}
//if (tmp.plate_score<100000)
if (tmp.plate_score<3)
	{
	tmp.isplate=true;
	Point2f a2(0, 0), b2(width, 0), c2(width, height),d2(0,height);
	point_extend (cn1, cn2, cn3, cn4,dis_extend);
	float dis2=norm(cn2-cn4);
	if (dis2==0)
		{
		dis2=width;
		}
	dis2=dis_extend*width/dis2;
	int extend=(int) (dis2);
	Mat plate_extended;
	Rect rect_small;
	rect_small.x=min(extend,width-1);
	rect_small.y=min(extend,height-1);
	rect_small.width=max(width-2*extend,0);
	rect_small.height=max(height-2*extend,0);
	tmp.plate_content= rect_small;
	cv::Point2f src[] = {cn1, cn3, cn4,cn2};
	cv::Point2f dst[] = {a2, b2, c2,d2};
	cv::Mat warpMat = cv::getAffineTransform(src, dst);
	cv::warpAffine(img_rgb,plate_extended, warpMat, img.size());
	tmp.plate_extended=plate_extended;
	//display_image(tmp.plate_extended);
	}
	
else
	{
	tmp.isplate=false;
	tmp.plate_extended = img_rgb;
	tmp.plate_content = Rect(region_extend,region_extend,width-2*region_extend,height-2*region_extend);
	tmp.cn1.x = region_extend+5;
	tmp.cn1.y = region_extend+5;
	}

return(tmp);
}