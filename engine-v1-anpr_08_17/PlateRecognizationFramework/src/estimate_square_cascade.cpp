#include <cstdio>
#include <cmath>
#include <iostream>
#include <vector>
#include <queue>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "IOData.h"
#include "estimate_square_cascade.h"



using namespace std;
using namespace cv;



cascade_input estimate_square_cascade(queue<long> car_estimate_square_cascade)
{
	int a[100000];
	int num_of_rang=20;
	float ratio = 2.1;
	int n=0;
	while (!car_estimate_square_cascade.empty())
	{
		n++;
		a[n] = car_estimate_square_cascade.front();
		car_estimate_square_cascade.pop();
	}
	int mina=a[1],maxa=a[1];
	int avg,sum=0;
	for (int i=1;i<=n;i++)
	{
		if (mina > a[i]) mina = a[i];
		if (maxa < a[i]) maxa = a[i];
		sum+=a[i];
	}
	avg = sum/n;
	int range = (maxa - mina)/(num_of_rang-1);
	int d[num_of_rang],gtd[num_of_rang];
	for (int i=0;i<num_of_rang;i++)
	{
		d[i]=0;
		gtd[i]=mina+range*i/2;
	}
	for (int i=1;i<=n;i++)
		d[(a[i]-mina)/range]++;
	for (int i=0;i<num_of_rang;i++)
		cout << d[i]<<" ";
	cout << endl;
	



	for (int i=0;i<num_of_rang;i++)
		cout << gtd[i]<<" ";
	cout << endl;
	/*int maxd=d[0],vtmaxd=0;
	for (int i=0;i<num_of_rang;i++)
		if (maxd<d[i])
		{
			maxd=d[i];
			vtmaxd=i;
		}
	int peak_moto,peak_car;
	if (gtd[vtmaxd]<avg)
	{
		peak_moto = gtd[vtmaxd];
		peak_car = (int)((float)peak_moto*ratio);
	}
	else
	{
		peak_car = gtd[vtmaxd];
		peak_moto = (int)((float)peak_car/ratio);
	}
	int peak_moto_real = peak_moto + 1/3*(peak_car-peak_moto);
	int peak_car_real = peak_car + 1/3*(peak_car-peak_moto);*/

	sum=0;
	for (int i=0;i<num_of_rang;i++)
		sum+=d[i];
	int left=0,right=num_of_rang-1;
	int sumleft=d[left];
	while (sumleft<sum*0.01)
	{
		left++;
		sumleft+=d[left];
	}
	int sumright = d[right];
	while (sumright<sum*0.01)
	{
		right--;
		sumright+=d[right];
	}
	int peak_moto_real = gtd[left+(right-left)/2];
	int peak_car_real = gtd[right];

	cascade_input result;
	float size_quare_cascade_x = atoi(IOData::GetCongfigData("size_quare_cascade_x:").c_str());
    float size_quare_cascade_y = atoi(IOData::GetCongfigData("size_quare_cascade_y:").c_str());
	float aa = (float)peak_moto_real;
	float bb = (float)peak_car_real;
	float carSquareMin_x = sqrt(peak_moto_real/size_quare_cascade_x/size_quare_cascade_y)*size_quare_cascade_x;
	float scale_moto_base = carSquareMin_x/size_quare_cascade_x;
	float sqrtn[10];
	for (int i=1;i<10;i++)
	{
		sqrtn[i] = pow(scale_moto_base,(float)1/i);
	}
	float nearest=abs(sqrtn[1]-sqrt(ratio));
	int vtnearest = 1;
	for (int i=2;i<10;i++)
		if (abs(sqrtn[i]-sqrt(ratio))<nearest)
		{
			nearest = abs(sqrtn[i]-sqrt(ratio));
			vtnearest = i;
		}
	result.detectScale = sqrtn[vtnearest];
	result.carSquareMin_x = (int)(size_quare_cascade_x*pow(result.detectScale,vtnearest)*0.9);
	result.carSquareMin_y = (int)(result.carSquareMin_x/size_quare_cascade_x*size_quare_cascade_y);
	result.carSquareMax_x = (int)(size_quare_cascade_x*pow(result.detectScale,vtnearest+1)*1.1);
	result.carSquareMax_y = (int)(result.carSquareMax_x/size_quare_cascade_x*size_quare_cascade_y);

	cout <<aa<<"\t"<<bb<<"\t"<< result.carSquareMin_x <<"\t"<<result.carSquareMin_y <<"\t"<<result.carSquareMax_x <<"\t"<<result.carSquareMax_y <<"\t"<<result.detectScale <<"\t"<<endl;
	//long plate
	float size_horizontal_cascade_x = atoi(IOData::GetCongfigData("size_long_cascade_x:").c_str());
    float size_horizontal_cascade_y = atoi(IOData::GetCongfigData("size_long_cascade_y:").c_str());
	
	result.carLongMin_x = sqrt((float)peak_car_real/size_quare_cascade_x/size_quare_cascade_y)*size_horizontal_cascade_x*0.9;
	result.carLongMin_y = sqrt((float)peak_car_real/size_quare_cascade_x/size_quare_cascade_y)*size_horizontal_cascade_y*0.9;
	result.carLongMax_x = sqrt((float)peak_car_real/size_quare_cascade_x/size_quare_cascade_y)*size_horizontal_cascade_x*1.1;
	result.carLongMax_y = sqrt((float)peak_car_real/size_quare_cascade_x/size_quare_cascade_y)*size_horizontal_cascade_y*1.1;
	result.detectScaleLong = (float)peak_car_real/size_quare_cascade_x/size_quare_cascade_y;
	return result;
}