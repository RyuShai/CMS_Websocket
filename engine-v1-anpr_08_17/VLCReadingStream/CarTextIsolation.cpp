#include "CarTextIsolation.h"
#include <opencv2/opencv.hpp>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "iostream"
#include "string"
#include "fstream"
#include "queue"
#include "math.h"
//#include "extra.cpp"
//#include "contoursF.cpp"



using namespace cv;
using namespace std;
using namespace pr;

const float rate_back_point_for_check_line_top=0.8;
const float rate_back_point_for_check_line_left=0.5;

void bwareaopen(cv::Mat& im, double size);
int check_character(Mat img,int left, int right, int up, int down);

struct space 
{
int start_point;
int end_point;
int mid_point;
};

Mat crop_image(int x,int y,int dx,int dy,Mat img)
{
	cv::Rect myROI(x, y, dx, dy);
	cv::Mat croppedRef(img, myROI);
	cv::Mat cropped;
	croppedRef.copyTo(cropped);
	return cropped;
}

bool isChar(Mat img)
{
	int n=img.size().height;
	int m=img.size().width;
	int black_point_top=0;
	int black_point_center=0;
	int black_point_bot=0;
	for (int i=0;i<n/3;i++)
		for (int j=0;j<m;j++)
			if ((int)img.at<uchar>(i,j)==0)
				black_point_top++;
	for (int i=n/3;i<2*n/3;i++)
		for (int j=0;j<m;j++)
			if ((int)img.at<uchar>(i,j)==0)
				black_point_center++;
	for (int i=2*n/3;i<n;i++)
		for (int j=0;j<m;j++)
			if ((int)img.at<uchar>(i,j)==0)
				black_point_bot++;
	int sum_black_point = black_point_bot+black_point_top+black_point_center;
	float rate_black_per_all = (float)sum_black_point/(float)n/(float)m;

	//if (black_point_bot==0 || black_point_top==0 || black_point_center==0||rate_black_per_all<=0.2)
	if (rate_black_per_all<=0.05)
		return false;
	return true;

}
void crop_element(Mat img,vector<cv::Rect>& charRegions,int eps,Mat img_bw)
{
	int n=img.size().height;
	int m=img.size().width;
	int a[n][m];
	int b[m+2];
	for (int i=0;i<m+2;i++)
		b[i]=0;
	for(int i=0;i<n;i++)
	{
		for (int j=0;j<m;j++)
		{
		a[i][j]=255-(int)img.at<uchar>(i,j);
	    if (a[i][j]==255)
	    	a[i][j]=1;
		}
	}	
	for (int i=0;i<m;i++)
	{
		for (int j=0;j<n;j++)
			b[i+1]+=a[j][i];
		double per = (double)b[i+1]/double(n);
		if (per<0.05)
			b[i+1]=0;
	}
	int crop_char_l[30],crop_char_r[30];
	int n_l=0,n_r=0;
	for (int i=1;i<m+1;i++)
	{
		if (b[i-1]==0 &&b[i]>0)
		{
			n_l++;
			crop_char_l[n_l]=i-1;
		}
		if (b[i+1]==0 &&b[i]>0)
		{
			n_r++;
			crop_char_r[n_r]=i-1;
		}
	}
	for (int i=1;i<=n_l;i++)
	{
		int distanceX = crop_char_r[i]-crop_char_l[i]+1;
		if ((float)distanceX/(float)n > 0.1)
		{
			int range_plus = 3;
			int x = crop_char_l[i];
			int y = crop_char_r[i]-crop_char_l[i]+1;
			int t=0;
			for (int j=1;j<=range_plus;j++)
				if (x>0)
				{
					x--;
					t++;
				}
			for (int j=1;j<=range_plus+t;j++)
				if (y+x<m-1)
					y++;
			Rect r1(x,0+eps, y, n);
			//cv::Rect r1(crop_char_l[i],0+eps, crop_char_r[i]-crop_char_l[i]+1, n);
			if (float(y)/float(n)>1.1)
			{
				r1 = Rect(x,0+eps, y/3, n);
				if (isChar(img_bw(r1)))
					charRegions.push_back(r1);
				r1 = Rect(x+y/3,0+eps, y/3, n);
				if (isChar(img_bw(r1)))
					charRegions.push_back(r1);
				r1 = Rect(x+2*y/3,0+eps, y/3, n);
				if (isChar(img_bw(r1)))
					charRegions.push_back(r1);
			}
			else if (float(y)/float(n)>0.7)
			{
				r1 = Rect(x,0+eps, y/2, n);
				if (isChar(img_bw(r1)))
					charRegions.push_back(r1);
				r1 = Rect(x+y/2,0+eps, y/2, n);
				if (isChar(img_bw(r1)))
					charRegions.push_back(r1);
			}
			else if (isChar(img_bw(r1)))
				charRegions.push_back(r1);
			//cout << x << endl;
		}
	}
}

int Threshold_bright(Mat img)
{
	int n=img.size().height;
	int m=img.size().width;
	//int a[n][m];
	int sum = 0;
	for(int i=0;i<n;i++)
		for (int j=0;j<m;j++)
			sum+=(int)img.at<uchar>(i,j);
	int avg1 = sum/(n*m);
	sum = 0;
	int num_pixcel=0;
	for(int i=0;i<n;i++)
			for (int j=0;j<m;j++)
				if ((int)img.at<uchar>(i,j)<=avg1)
				{
					sum+=(int)img.at<uchar>(i,j);
					num_pixcel++;
				}
	int avg2 = sum/num_pixcel;
	return avg2;
}
int Threshold_bright_fillchar(Mat img)
{
	int n=img.size().height;
	int m=img.size().width;
	//int a[n][m];
	int sum = 0;
	for(int i=0;i<n;i++)
		for (int j=0;j<m;j++)
			sum+=(int)img.at<uchar>(i,j);
	int avg1 = sum/(n*m);
	/*sum = 0;
	int num_pixcel=0;
	for(int i=0;i<n;i++)
			for (int j=0;j<m;j++)
				if ((int)img.at<uchar>(i,j)<=avg1)
				{
					sum+=(int)img.at<uchar>(i,j);
					num_pixcel++;
				}
	int avg2 = sum/num_pixcel;*/
	return avg1;
}

int Threshold_bright_delete_line(Mat img)
{
	int n=img.size().height;
	int m=img.size().width;
	//int a[n][m];
	int sum = 0;
	for(int i=0;i<n;i++)
		for (int j=0;j<m;j++)
			sum+=(int)img.at<uchar>(i,j);
	int avg1 = sum/(n*m);
	/*sum = 0;
	int num_pixcel=0;
	for(int i=0;i<n;i++)
			for (int j=0;j<m;j++)
				if ((int)img.at<uchar>(i,j)<=avg1)
				{
					sum+=(int)img.at<uchar>(i,j);
					num_pixcel++;
				}
	int avg2 = sum/num_pixcel;*/
	return avg1;
}

std::vector<cv::Rect> getCharacterRect_AThanh(cv::Mat& img_rgb, int mode){
	std::vector<cv::Rect> charRegions;
	Mat img_gray,img_bw;
	cvtColor(img_rgb, img_gray,CV_RGB2GRAY);
	threshold(img_gray, img_bw, Threshold_bright(img_gray), 255.0, THRESH_BINARY);
	int n=img_rgb.size().height;
	int m=img_rgb.size().width;
	Mat img_top = crop_image(0, 0, m, n/2,img_bw);
	crop_element(img_top,charRegions,0,img_bw);
	Rect r1(0,0, 0, 0);
	charRegions.push_back(r1);
	Mat img_bot = crop_image(0, n/2, m, n/2,img_bw);
	crop_element(img_bot,charRegions,n/2,img_bw);
	return charRegions;
}



//================= Thanh edit ================================================

double pointdistance(cv::Point a1, cv::Point a2)
{
	return (pow((a2.x - a1.x), 2) + pow((a2.y - a1.y), 2));
}

Mat convert_image_bw(Mat img)
{
	cv::cvtColor(img, img, CV_BGR2GRAY);
	bitwise_not (img, img);
	adaptiveThreshold(img, img, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, 15, 8);
	return img;
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
Point intersection(Point p1,Point p2,Point p3, Point p4)
{
	Point pt;
	pt.x=((p1.x*p2.y-p1.y*p2.x)*(p3.x-p4.x)-(p1.x-p2.x)*(p3.x*p4.y-p3.y*p4.x))/((p1.x-p2.x)*(p3.y-p4.y)-(p1.y-p2.y)*(p3.x-p4.x));
	pt.y=((p1.x*p2.y-p1.y*p2.x)*(p3.y-p4.y)-(p1.y-p2.y)*(p3.x*p4.y-p3.y*p4.x))/((p1.x-p2.x)*(p3.y-p4.y)-(p1.y-p2.y)*(p3.x-p4.x));
	return (pt);
	}

	int croping(Mat img,Point &cn1, Point &cn2, Point &cn3, Point &cn4,float ratio) //src: input gray image, img: the binary image after thresholding
	{
	Mat img_canny;
	Canny(img,img_canny,50,150,3);
	int height=img.size().height;
	int width=img.size().width;	
	//Mat cdst;
	//cvtColor(img, cdst, CV_GRAY2BGR);
	//cvtColor(src, src, CV_GRAY2BGR);
	vector<Vec2f> lines;
	vector<Vec2f> lines_hu;
	vector<Vec2f> lines_hd;
	vector<Vec2f> lines_vl;
	vector<Vec2f> lines_vr;

	int thres_vote=floor(height/3);

	HoughLines(img_canny,lines, 1, CV_PI/180,thres_vote, 0, 0 );

	Point pt1,pt2,pt3,pt4,pt5,pt6; //pt3,pt4,pt5,pt6 are 4 points at conrner of the image
	pt3.x=0; pt3.y=0; //top left corner;
	pt4.x=0; pt4.y=height-1;// bottom left corner;
	pt5.x=width-1;pt5.y=0;//top right cornenr
	pt6.x=width-1;pt6.y=height-1;//bottom right corner
	float interthres=0.4; //threshold to decide if a lines belong to hu,hd,vl,vr or not 
	for (int i=0;i<(int)lines.size();i++)
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
		if ( theta>CV_PI/180*150 || theta<CV_PI/180*30)
		{
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
		else if ( theta>CV_PI/180*60 && theta<CV_PI/180*120)
			{
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
	int num_lines=0;
	float rho,theta;
	double a,b,x0,y0;
	Point pt11,pt12,pt21,pt22,pt31,pt32,pt41,pt42; //the cordinate of 4 lines_end points
	int type=0; //type =1:miss vl, type =2: miss vr, type =3: miss hu, type =4: miss hd

	//----------check the vertical left-----------------
	if (lines_vl.size()>0)
	{
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
	//----------check the vertical right-----------------
	if (lines_vr.size()>0)
	{
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

	//----------check the horizontal up-----------------
	if (lines_hu.size()>0)
	{
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

	//----------check the horizontal down-----------------
	if (lines_hd.size()>0)
	{
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
	//printf("number of line is %d  type=%d \n", num_lines,type);
	lines_vl.clear();
	lines_vr.clear();
	lines_hu.clear();
	lines_hd.clear();

	if (num_lines==4)
	{
		cn1=intersection(pt11,pt12,pt31,pt32);
		cn2=intersection(pt11,pt12,pt41,pt42);
		cn3=intersection(pt31,pt32,pt21,pt22);
		cn4=intersection(pt41,pt42,pt21,pt22);
	}
	float d1,d2,delta_x,delta_y;
	if (num_lines==3)
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
		}

	return (num_lines);
}
//-----------------------------Extend Point------------------------------
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
//-----------------------------------------------------------------------------------

float check_score(Vec2f can_vl,Vec2f can_vr,Vec2f can_hu,Vec2f can_hd)
{
	Point pt11,pt12,pt21,pt22,pt31,pt32,pt41,pt42; //the cordinate of 4 lines_end points
	double a,b,x0,y0;
	float v_distance=0; // the distance of angle between 2 vertical lines
	float h_distance=0; // the distance of angle between 2 horizontal lines
	float ratio_distance=0;// the distance in ration (in compare with the standard plate
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
	//----------------------------------------------
	if ((angle_vr!=0)||(angle_vl!=0))
		{
		v_distance=abs(angle_vl-angle_vr)/max(abs(angle_vl),abs(angle_vr));
		}
	else
		{
		v_distance=0;
		}
	if ((angle_hu!=0)||(angle_hd!=0))
		{
		h_distance=abs(angle_hu-angle_hd)/max(abs(angle_hu),abs(angle_hd));
		}
	else
		{
		h_distance=0;
		}
	float ratio_car=1.4;
	float ratio_bike=1.357;
	float d_vl=norm(cn1-cn2);//length of vertical left line
	float d_vr=norm(cn3-cn4);//length of vertical right line
	float d_hu=norm(cn1-cn3);//length of horizontal up line
	float d_hd=norm(cn2-cn4);//length of horizontal down line
	float ratio=(d_hu+d_hd)/(d_vl+d_vr);
	ratio_distance=min(abs(ratio-ratio_car)/ratio_car, abs(ratio-ratio_bike)/ratio_bike);
	float total_distance;
	total_distance=v_distance+h_distance+ratio_distance*2;
	//printf("v=%f  h=%f  r=%f final =%f\n",v_distance,h_distance,ratio_distance,total_distance);
	return(total_distance);
}
float croping_v2(Mat img,Point &cn1, Point &cn2, Point &cn3, Point &cn4,float ratio)
{
	Mat img_canny;
	Canny(img,img_canny,50,150,3);
	int height=img.size().height;
	int width=img.size().width;
	vector<Vec2f> lines; //vector that stores all the lines detected by Hough Transform
	vector<Vec2f> lines_hu; // vector that stores all the candidates for the upper horizontal line 
	vector<Vec2f> lines_hd;// vector that stores all the candidates for the lower horizontal line
	vector<Vec2f> lines_vl;// vector that stores all the candidates for the left vertical line
	vector<Vec2f> lines_vr;// vector that stores all the candidates for the right vertical line 
	int thres_vote=floor(height/4);// minimum votes for a line (in Hough transform)
	HoughLines(img_canny,lines, 1, CV_PI/180,thres_vote, 0, 0 ); // Hough transform to detect lines whose votes> thres_vote. All lines are saved into vector lines
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
		if ( theta>CV_PI/180*160 || theta<CV_PI/180*20)
		{
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
		else if ( theta>CV_PI/180*70 && theta<CV_PI/180*110)
			{
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

	//----------------- End of line detection------------------------------------------
	//------------------Preparation to find the best set of lines to form the license plate-----------
	int max_line=4; // We check "max_line" number of candidate from each vector
	int num_vl; // The actual number of vertical left to be checked, this could be different from max_line if lines_vl.size()<max_line
	int num_vr;
	int num_hu;
	int num_hd;
	int num_lines=0;//Number of lines that can be detected 
	float rho,theta;
	double a,b,x0,y0;
	Point pt11,pt12,pt21,pt22,pt31,pt32,pt41,pt42; //the cordinate of 4 lines_end points
	int type=0; //type =1:miss vl, type =2: miss vr, type =3: miss hu, type =4: miss hd
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
	if (num_lines==4)
		{
		plate_score=999; //the score that determine how likely the candidates could form a plate. The smaller score is, the more confident we have
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
						float score_tmp=check_score(can_vl,can_vr,can_hu,can_hd);
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
		plate_score=0.5;
		}
	else
		{
		plate_score=20;
		}
	return(plate_score);
}

Mat thresholding (Mat img)
{
	Mat img_t1; 
	adaptiveThreshold(img, img_t1, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 5,0 );
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

void clean_line_around_plate (Mat &img_rgb)
{
	int n=img_rgb.size().height;
	int m=img_rgb.size().width;
	Mat img_gray,img_bw;
	cvtColor(img_rgb, img_gray,CV_RGB2GRAY);
	threshold(img_gray, img_bw, Threshold_bright_delete_line(img_gray), 255.0, THRESH_BINARY);
	//line_top
	bool kn=true;
	//line_bot
	kn=true;
	while (kn && n>40)
	{
		int black_point=0;
		for (int i=0;i<m;i++)
			if ((int)img_bw.at<uchar>(n-1,i)==0)
				black_point++;
		if ((float)black_point/(float)m>rate_back_point_for_check_line_top)
		{
			img_bw = crop_image(0, 0, m, n-1,img_bw);
			img_rgb = crop_image(0, 0, m, n-1,img_rgb);
			--n;
		}
		else kn=false;
	}
	//line_left
	kn=true;
	while (kn && m>40)
	{
		int black_point=0;
		for (int i=0;i<n;i++)
			if ((int)img_bw.at<uchar>(i,0)==0)
				black_point++;
		if ((float)black_point/(float)n>rate_back_point_for_check_line_left)
		{
			img_bw = crop_image(1, 0, m-1, n,img_bw);
			img_rgb = crop_image(1, 0, m-1, n,img_rgb);
			--m;
		}
		else kn=false;
	}
	//line_left
	kn=true;
	while (kn && m>40)
	{
		int black_point=0;
		for (int i=0;i<n;i++)
			if ((int)img_bw.at<uchar>(i,m-1)==0)
				black_point++;
		if ((float)black_point/(float)n>rate_back_point_for_check_line_left)
		{
			img_bw = crop_image(0, 0, m-1, n,img_bw);
			img_rgb = crop_image(0, 0, m-1, n,img_rgb);
			--m;
		}
		else kn=false;
	}
	//imshow("a",img);
}
std::string getCurrentDateTime1()
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

Mat CropPlate_ATon(cv::Mat& img,float & rate_crop_4point,Rect& Rect_small_CropPlate)
{
	cv::Mat scr;
	img.copyTo(scr);
	int n=img.size().height;
	int m=img.size().width;
	cvtColor(img, img,CV_RGB2GRAY);
	Mat img1=thresholding(img);
	float ratio;
	//ratio=4.27; // bien oto dai
	ratio=1.38; // bien oto 
	//ratio=1.357; // bien xe may
	Point cn1, cn2, cn3, cn4;
	//float result; //result = number of line detected
	//result= croping(img1,cn1,cn2,cn3,cn4,ratio);
	rate_crop_4point= croping_v2(img1,cn1,cn2,cn3,cn4,ratio);
	cv::Point2f a2(0, 0), b2(m, 0), c2(m, n),d2(0,n);
	//print square plate
	/*
	if (rate_crop_4point<=0.1)
	{
		Point c1,cc2,c3,c4;
		c1=cn1; cc2=cn2; c3=cn3; c4=cn4;
		int rang=20;
		c1.x-=rang;c1.y-=rang;cc2.x-=rang;cc2.y+=rang;c3.x+=rang;c3.y-=rang;c4.x+=rang;c4.y+=rang;
		Mat img_abc = img.clone();
		cv::Point2f srcp[] = {c1, c3, c4,cc2};
		cv::Point2f dstp[] = {a2, b2, c2,d2};
		cv::Mat warpMatp = cv::getAffineTransform(srcp, dstp);
		cv::warpAffine(img_abc,img_abc, warpMatp, img_abc.size());
		//string link =  "/home/vp9-anpr/Desktop/Result/plate_beatiful_square/"+getCurrentDateTime1()+".jpg";
		imwrite(link,img_abc);
	}
	*/
	//square plate
	float dis;
	dis=10;
	point_extend (cn1, cn2, cn3, cn4,dis);
	float dis2=norm(cn2-cn4);
	dis2=dis*m/dis2;
	int extend=(int) (dis2);
	Rect rect_small={extend,extend,m-2*extend,n-2*extend};
	Rect_small_CropPlate = rect_small;
	cv::Point2f src[] = {cn1, cn3, cn4,cn2};
	cv::Point2f dst[] = {a2, b2, c2,d2};
	cv::Mat warpMat = cv::getAffineTransform(src, dst);
	cv::warpAffine(scr,scr, warpMat, scr.size());
	//clean_line_around_plate(scr);
	
	return scr;
}


//============================================== Car Horizol Plate ========================
int Threshold_bright_Horizol(Mat img)
{
	int n=img.size().height;
	int m=img.size().width;
	//int a[n][m];
	int sum = 0;
	for(int i=0;i<n;i++)
		for (int j=0;j<m;j++)
			sum+=(int)img.at<uchar>(i,j);
	int avg1 = sum/(n*m);
	sum = 0;
	int num_pixcel=0;
	for(int i=0;i<n;i++)
			for (int j=0;j<m;j++)
				if ((int)img.at<uchar>(i,j)>=avg1)
				{
					sum+=(int)img.at<uchar>(i,j);
					num_pixcel++;
				}
	int avg2 = sum/num_pixcel;
	return ((avg2+avg1)/2+avg1)/2;
}
Mat convert_image_bw_Horizol(Mat img)
{
	cv::cvtColor(img, img, CV_BGR2GRAY);
	threshold(img, img, Threshold_bright_Horizol(img), 255.0, THRESH_BINARY);
	return img;
}
Mat CropPlate_Horizol_AThanh(cv::Mat& img)
{
	queue <Point> myqueue;
	int dx[4]={-1,0,1,0};
	int dy[4]={0,1,0,-1};
	//int pictures_input = 3983;
	cv::Mat scr=img;
	int n=img.size().height;
	int m=img.size().width;
	img = convert_image_bw_Horizol(img);
	img = fill_holes(img);
	int a[n][m],check[n][m];
	    for(int i=0;i<n;i++)
	    	for (int j=0;j<m;j++)
	    	{
	    		a[i][j]=(int)img.at<uchar>(i,j);
	    		check[i][j]=1;
	      }
	    Point p,q;
		p.x = n/2;
		p.y = m/2;
		int eps_range=5;
		for (int i=p.x-eps_range;i<=p.x+eps_range;i++)
			for (int j=p.y-eps_range;j<=p.y+eps_range;j++)
				if (a[i][j]==255)
				{
					q.x=i;q.y=j;
					check[i][j]=0;
					myqueue.push(q);
				}

		while (!myqueue.empty())
		{
			q=myqueue.front();
			myqueue.pop();
			for (int i=0;i<4;i++)
			{
				Point td;
				td.x = q.x+dx[i];
				td.y = q.y+dy[i];
				if ((check[td.x][td.y]==1) && (a[td.x][td.y]==255))
				{
					myqueue.push(td);
					check[td.x][td.y]=0;
				}
			}
		}

		for (int i=0;i<n;i++)
			for (int j=0;j<m;j++)
				if (check[i][j]==0)
					img.at<uchar>(i,j) = 255;
				else img.at<uchar>(i,j) = 0;
		
		double mintl = 99999;
		double mintr = 99999;
		double minbl = 99999;
		double minbr = 99999;
		Point tl, tr, bl, br;
		vector<Point2f> inputQuad, outputQuad;
		for (int i=0;i<n;i++)
			for (int j=0;j<m;j++)
				if (check[i][j]==0)
				{
					double dtl = pointdistance(Point(i,j), Point(0, 0));
					double dtr = pointdistance(Point(i,j), Point(0, m));
					double dbl = pointdistance(Point(i,j), Point(n, 0));
					double dbr = pointdistance(Point(i,j), Point(n, m));
		    		if (dtl < mintl) {
		    			mintl = dtl;
		    			tl = Point(i,j);
		    		}
		    		if (dtr < mintr) {
		    			mintr = dtr;
		    			tr = Point(i,j);
		    		}
		    		if (dbl < minbl) {
		    			minbl = dbl;
		    			bl = Point(i,j);
		    		}
		    		if (dbr < minbr) {
		    			minbr = dbr;
		    			br = Point(i,j);
		    		}
				}
		cv::Point2f a1(tl.y, tl.x), b1(tr.y, tr.x), c1(br.y, br.x),d1(bl.x,bl.y), a2(0, 0), b2(m, 0), c2(m, n),d2(0,n);
		cv::Point2f src[] = {a1, b1, c1,d1};
		cv::Point2f dst[] = {a2, b2, c2,d2};
		cv::Mat warpMat = cv::getAffineTransform(src, dst);
		cv::warpAffine(scr,img, warpMat, img.size());
		clean_line_around_plate(img);
		return img;
}
//================================= Code crop 4 goc bien dai anh Ton ============================

float check_score_Horizol(Vec2f can_vl,Vec2f can_vr,Vec2f can_hu,Vec2f can_hd)
{
	Point pt11,pt12,pt21,pt22,pt31,pt32,pt41,pt42; //the cordinate of 4 lines_end points
	double a,b,x0,y0;
	float v_distance=0; // the distance of angle between 2 vertical lines
	float h_distance=0; // the distance of angle between 2 horizontal lines
	float ratio_distance=0;// the distance in ration (in compare with the standard plate
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
	//----------------------------------------------
	if ((angle_vr!=0)||(angle_vl!=0))
		{
		v_distance=abs(angle_vl-angle_vr)/max(abs(angle_vl),abs(angle_vr));
		}
	else
		{
		v_distance=0;
		}
	if ((angle_hu!=0)||(angle_hd!=0))
		{
		h_distance=abs(angle_hu-angle_hd)/max(abs(angle_hu),abs(angle_hd));
		}
	else
		{
		h_distance=0;
		}
	//float ratio_car=1.4;
	//float ratio_bike=1.357;
	float ratio_long=4.27;
	float d_vl=norm(cn1-cn2);//length of vertical left line
	float d_vr=norm(cn3-cn4);//length of vertical right line
	float d_hu=norm(cn1-cn3);//length of horizontal up line
	float d_hd=norm(cn2-cn4);//length of horizontal down line
	float ratio=(d_hu+d_hd)/(d_vl+d_vr);
	ratio_distance=abs(ratio-ratio_long)/ratio_long;
	float total_distance;
	total_distance=v_distance+h_distance+ratio_distance*2;
	//printf("v=%f  h=%f  r=%f final =%f\n",v_distance,h_distance,ratio_distance,total_distance);
	return(total_distance);
}

float croping_Horizol(Mat img,Point &cn1, Point &cn2, Point &cn3, Point &cn4,float ratio)
{
	Mat img_canny;
	Canny(img,img_canny,50,150,3);
	int height=img.size().height;
	int width=img.size().width;
	vector<Vec2f> lines; //vector that stores all the lines detected by Hough Transform
	vector<Vec2f> lines_hu; // vector that stores all the candidates for the upper horizontal line 
	vector<Vec2f> lines_hd;// vector that stores all the candidates for the lower horizontal line
	vector<Vec2f> lines_vl;// vector that stores all the candidates for the left vertical line
	vector<Vec2f> lines_vr;// vector that stores all the candidates for the right vertical line 
	int thres_vote=floor(height/4);// minimum votes for a line (in Hough transform)
	HoughLines(img_canny,lines, 2, CV_PI/180,thres_vote, 0, 0 ); // Hough transform to detect lines whose votes> thres_vote. All lines are saved into vector lines
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
		if ( theta>CV_PI/180*160 || theta<CV_PI/180*20)
		{
		interthres=0.3;
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
		else if ( theta>CV_PI/180*70 && theta<CV_PI/180*110)
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

	//----------------- End of line detection------------------------------------------
	//------------------Preparation to find the best set of lines to form the license plate-----------
	int max_line=3; // We check "max_line" number of candidate from each vector
	int num_vl; // The actual number of vertical left to be checked, this could be different from max_line if lines_vl.size()<max_line
	int num_vr;
	int num_hu;
	int num_hd;
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
	max_line=3;
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
	if (num_lines==4)
		{
		plate_score=999; //the score that determine how likely the candidates could form a plate. The smaller score is, the more confident we have
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
						float score_tmp=check_score_Horizol(can_vl,can_vr,can_hu,can_hd);
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
		plate_score=0.5;
		}
	else
		{
		plate_score=20;
		}
		//printf("end\n");
	return(plate_score);
}

Mat CropPlate_ATon_Horizol(cv::Mat& img,float& rate_crop_4point)//,float & rate_crop_4point)
{

	cv::Mat scr;
	img.copyTo(scr);
	int n=img.size().height;
	int m=img.size().width;
	cvtColor(img, img,CV_RGB2GRAY);
	Mat img1=thresholding(img);
	float ratio;
	ratio=1.38; // bien oto 
	Point cn1, cn2, cn3, cn4;
	rate_crop_4point= croping_Horizol(img1,cn1,cn2,cn3,cn4,ratio);
	cv::Point2f a2(0, 0), b2(m, 0), c2(m, n),d2(0,n);
	/*
	if (rate_crop_4point<=0.1)
	{
		Point c1,cc2,c3,c4;
		c1=cn1; cc2=cn2; c3=cn3; c4=cn4;
		int rang=4;
		c1.x-=rang;c1.y-=rang;cc2.x-=rang;cc2.y+=rang;c3.x+=rang;c3.y-=rang;c4.x+=rang;c4.y+=rang;
		Mat img_abc = img.clone();
		cv::Point2f srcp[] = {c1, c3, c4,cc2};
		cv::Point2f dstp[] = {a2, b2, c2,d2};
		cv::Mat warpMatp = cv::getAffineTransform(srcp, dstp);
		cv::warpAffine(img_abc,img_abc, warpMatp, img_abc.size());
		//string link =  "/home/vp9-anpr/Desktop/Result/plate_beatiful_long/"+getCurrentDateTime1()+".jpg";
		imwrite(link,img_abc);
	}*/
	

	//square plate


	cv::Point2f src[] = {cn1, cn3, cn4,cn2};
	cv::Point2f dst[] = {a2, b2, c2,d2};
	cv::Mat warpMat = cv::getAffineTransform(src, dst);
	cv::warpAffine(scr,scr, warpMat, scr.size());
	clean_line_around_plate(scr);
	return scr;
}
//End code a Ton===================================================================





std::vector<cv::Rect> getCharacterRect_Horizol_AThanh(cv::Mat& img_rgb, int mode){
	std::vector<cv::Rect> charRegions;
	Mat img_gray,img_bw;
	cvtColor(img_rgb, img_gray,CV_RGB2GRAY);
	threshold(img_gray, img_bw, Threshold_bright(img_gray), 255.0, THRESH_BINARY);
	crop_element(img_bw,charRegions,0,img_bw);
	return charRegions;
}

//Code crop char bien dai a Ton==================================================================================

std::vector<cv::Rect> getCharacterRect_LongPlate(cv::Mat& img_abc, int mode)
{
	std::vector<cv::Rect> charRegions;
	Mat img,img_rgb, img_t1,img_t2,img_bw; // img: gray image; img_t1: binary image
	img_rgb = img_abc.clone();
	cvtColor(img_rgb, img_rgb,CV_RGB2GRAY);
	int height_ori=img_rgb.size().height; // height of the original image
	int width_ori=img_rgb.size().width;  // width of the original image
	//prepare another binay image for filter out the non-character REC
	img_t2=img_rgb;
	img=img_rgb;
	cv::resize(img_t2, img_t2, cv::Size(376,88), 0, 0, CV_INTER_LINEAR);
	equalizeHist( img_t2, img_t2);
	bitwise_not(img_t2,img_t2);
	adaptiveThreshold(img_t2, img_bw, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 21,-15);
	bwareaopen(img_bw,200);
	bitwise_not(img_bw,img_bw);
	//---Pre processing the image with resize, equalize---
	cv::resize(img, img, cv::Size(376,88), 0, 0, CV_INTER_LINEAR);
	equalizeHist( img, img);
	bitwise_not(img,img);
	adaptiveThreshold(img, img_t1, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 21,1);
	bwareaopen(img_t1,500); // Delete small object
	bitwise_not(img_t1,img_t1);
	//display_image(img_t1);
	//--------------------Begin to process the image------------------------------------------
	int height=img.size().height;  // height of the resized image (160)
	int width=img.size().width;    // widthe of the resized image (216)
	int hist_h[400]; // projection of all pixel values on the horizontal direction
	int hist_hh[400];// sum over 3 pixels of hist_h;
	int project[400]; // projection of all pixel values on the vertical direction 
	float cw=0.87;
	int g_max=0; // the maximum value of projection, this is used as a threshold to define if a pixel is lie on a space or a character
	float ratio_width=(float) width_ori/width;
	//---------------------------crop character of the upper regions---------------------------------------------------------------------------------
	for (int i=0;i<width;i++)
		{
		project[i]=0;
		}
	for (int i=0;i<width;i++)
		{
		for (int j=0;j<height;j++)
		{
		project[i]=project[i]+img_t1.at<uchar>(j, i);
		}
		}
	vector <space> v_space;
	int max_index=0;
	int max=0;
	for (int i=0;i<width;i++)
	{
		if (g_max<project[i])
		{
		g_max=project[i];
		}
		} 
	//-------------Calculate the potential space------------\97
	int positive[400];
	int num_positive;
	positive[0]=0;//the position of the first space candidate
	num_positive=1; 
	for (int i=1;i<width-1;i++)
	{
	if (project[i]>cw*g_max)
		{
		positive[num_positive]=i;
		num_positive++;
		}
	}
	positive[num_positive]=width-1; //the position of the last space candidate
	num_positive++;
	int enlarge;
	for (int i=1;i<num_positive;i++)
	{
	space tmp;
	//printf("i=%d positive[i-1] =%d\n",i,positive[i-1]);
	//int width_thres;
	if  ((positive[i-1]>width*0.212)&&(positive[i-1]<width*0.346)) // the position of the character in the upper region, we use bigger threshold for character (it can not be number '1')
		{
		enlarge=5;
		if ((positive[i]-positive[i-1])>25)
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
		if ((positive[i]-positive[i-1])>12)
		{
	//    printf("i=%d positive[i-1] =%d,positive[i]=%d\n",i,positive[i-1],positive[i]);

			enlarge=5;
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

	int start_point,end_point;// position of the character
	for (int i=0;i<v_space.size();i++)
	{
	//printf("number of character is %d\n",v_space.size());
	int num_region=0;
	if ((v_space[i].end_point-v_space[i].start_point)<75)
		{
		num_region=1;
		}
	else if ((v_space[i].end_point-v_space[i].start_point)<105)
		{
		num_region=2;
		}
	else if((v_space[i].end_point-v_space[i].start_point)<135)
		{
		num_region=3;
		}
	else if((v_space[i].end_point-v_space[i].start_point)<185)
		{
		num_region=4;
		}
	else
		num_region=5;
	// num_region=1;
	if (num_region==1) // xu li cho 1 region
		{
		if (check_character(img_bw,v_space[i].start_point,v_space[i].end_point,0,height)==1)
		{
		start_point=(int) (ratio_width*v_space[i].start_point);
		end_point=(int) (ratio_width*v_space[i].end_point);
		//  printf("end=%d, start=%d\n",end_point,start_point);
		Rect Rec(start_point, 0, end_point-start_point,height_ori);
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
		for (int k=mid_point-12;k<mid_point+12;k++)
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
		//  printf("region %d\n",j);
		if (check_character(img_bw,cut_point[j],cut_point[j+1],0,height)==1)
			{
	//     printf("start=%d, end=%d \n",cut_point[j],cut_point[j+1]);
			start_point=(int) (ratio_width*cut_point[j]);
			end_point=(int) (ratio_width*cut_point[j+1]);
			Rect Rec(start_point, 0, end_point-start_point,height_ori-1);
			charRegions.push_back(Rec);
			}
		}
		}
	}
	v_space.clear();

	return charRegions;
}

//End code a Ton -------------------------------------------------------/




//code a Ton ==================================================================================




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

std::vector<cv::Rect> getCharacterRect_ATon(cv::Mat& img_rgb, int mode)
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
	max=0;
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
	max=0;
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
//end code a Ton================================================================================
//Code Thanh fill ky tu thua====================================================================
std::vector<cv::Rect> fillchar(cv::Mat& img_rgb, vector<cv::Rect> charRegions){
	
	Mat img_gray,img_bw;
	cvtColor(img_rgb, img_gray,CV_RGB2GRAY);
	threshold(img_gray, img_bw, Threshold_bright_fillchar(img_gray), 255.0, THRESH_BINARY);	
	vector<cv::Rect> charRects;
	for (int i = 0; i < (int)charRegions.size(); i++) {
		if (isChar(img_bw(charRegions[i])) || (charRegions[i].x==0 && charRegions[i].y==0 && charRegions[i].width==0 && charRegions[i].height==0))
			charRects.push_back(charRegions[i]);
	}
	return charRects;
}
//end code Thanh
//Code Bach tach vien =====================
/*
cv::Mat RemoveBorders(cv::Mat im_rgb)
{
	cv::Rect CRECT = cropR(im_rgb);
	cv::Mat output;
	im_rgb(CRECT).copyTo(output);
	return output;
}*/

//=============================================

std::vector<cv::Rect> CarTextIsolation::getCharacterRect(cv::Mat& img_rgb, int mode){
	//img_rgb = RemoveBorders(img_rgb);
	//return getCharacterRect_AThanh(img_rgb, mode);
	return getCharacterRect_ATon(img_rgb, mode);
	//return fillchar(img_rgb,getCharacterRect_ATon(img_rgb, mode));

}
std::vector<cv::Rect> CarTextIsolation::getCharacterRect_Horizol(cv::Mat& img_rgb, int mode){
	//return getCharacterRect_Horizol_AThanh(img_rgb, mode);
	return getCharacterRect_LongPlate(img_rgb, mode);
}
Mat CarTextIsolation::CropPlate_Horizol(cv::Mat& img,float& rate_crop_4point)
{
	//return CropPlate_Horizol_AThanh(img);
	return CropPlate_ATon_Horizol(img,rate_crop_4point);
}
Mat CarTextIsolation::CropPlate(cv::Mat& img,float& rate_crop_4point,Rect& Rect_small_CropPlate)
{
	return CropPlate_ATon(img,rate_crop_4point,Rect_small_CropPlate);

}