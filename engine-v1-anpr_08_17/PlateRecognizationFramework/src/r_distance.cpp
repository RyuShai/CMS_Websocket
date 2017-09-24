#include <cstdio>
#include <cmath>
#include <iostream>
#include <vector>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"


using namespace std;
using namespace cv;

// endgrid 0 960 
// divgrid 1540 -780.012 
// l1 960 -1.12988 
// v1 1640 -893 
// v2 -24199 -700 
void display(vector<double> v)
{
	for (int i = 0, j = v.size(); i < j; i++)
	{
		cout << v[i] << " ";
	}
	cout << "\n";
}

vector<double> get_line(vector<double> point1, vector<double> point2)
{
	vector<double> result;

	double x1 = point1[0];
	double y1 = point1[1];
	double x2 = point2[0];
	double y2 = point2[1];

	double b = (y2 - y1)/(x2 - x1);
	double a = y1 - b*x1;
	
	result.push_back(a);
	result.push_back(b);
	return result;
}

vector<double> get_point(vector<double> line1, vector<double> line2)
{
	vector<double> result;

	double a1 = line1[0];
	double b1 = line1[1];
	double a2 = line2[0];
	double b2 = line2[1];

	double x = float(a2 - a1)/(b1 - b2);
	double y = b1*x + a1;

	result.push_back(x);
	result.push_back(y);
	return result;
}

double distance(vector<double> point1, vector<double> point2)
{
	double x1 = point1[0];
	double y1 = point1[1];
	double x2 = point2[0];
	double y2 = point2[1];

	return sqrt(pow((x2 - x1), 2) + pow((y2 - y1), 2));
}

vector<double> get_mid(vector<double> end1, vector<double> end2, vector<double> l1, vector<double> v1, vector<double> v2)
{
	vector<double> testp;
	testp.push_back(600);
	testp.push_back(800);

	vector<double> lp = get_line(v1, testp);

	vector<double> le1 = get_line(v2, end1);
	vector<double> le2 = get_line(v2, end2);

	vector<double> inter1 = get_point(le1, lp);
	vector<double> inter2 = get_point(le2, lp);

	vector<double> crossL = get_line(end1, inter2);
	vector<double> crossR = get_line(end2, inter1);

	vector<double> middle = get_point(crossL, crossR);

	vector<double> finsec = get_line(middle, v2);

	vector<double> midres = get_point(finsec, l1);

	return midres;
	// return testp;
}

double bin_cstr(vector<double> begin, vector<double> end, vector<double> tpoint, int iteration, vector<double> l1, vector<double> v1, vector<double> v2)
{
	vector<double> mid = get_mid(begin, end, l1, v1, v2);
	// cout << "begin" << endl;
	// display(begin);
	// cout << "end" << endl;
	// display(end);
	// cout << "tpoint" << endl;
	// display(tpoint);

	if (distance(mid, tpoint) < 0.5)
		return pow(0.5, iteration);
	else
	{
		double result = 0;
		if (abs(distance(mid, begin) - distance(mid, tpoint) - distance(begin, tpoint)) < 0.5)
			result = pow(0.5, iteration) + bin_cstr(begin, mid, tpoint, iteration + 1, l1, v1, v2);
		else
			result = bin_cstr(mid, end, tpoint, iteration + 1, l1, v1, v2);
		return result;
	}
}

double x_dist_proj(vector<double> point1, vector<double> point2, vector<double> endgrid, vector<double> divgrid, vector<double> l1, vector<double> v1, vector<double> v2)
{
	vector<double> l12 = get_line(point1, v2);
	vector<double> l22 = get_line(point2, v2);

	vector<double> l_prime = get_line(endgrid, divgrid);

	vector<double> int1 = get_point(l12, l_prime);
	vector<double> int2 = get_point(l22, l_prime);

	double r1 = bin_cstr(endgrid, divgrid, int1, 1, l1, v1, v2);
	double r2 = bin_cstr(endgrid, divgrid, int2, 1, l1, v1, v2);

	return abs(r2 - r1);
}

double rw_dist(vector<double> p1, vector<double> p2, vector<double> endgrid, vector<double> divgrid, vector<double> l1, vector<double> v1, vector<double> v2, double scale)
{
	return x_dist_proj(p1, p2, endgrid, divgrid, l1, v1, v2)*scale;
}

double d_dist(double p11x,double p11y,double p22x,double p22y)
{
	vector<double> p1, p2;
	p1.push_back(p11x);
	p1.push_back(p11y);
	p2.push_back(p22x);
	p2.push_back(p22y);
	
	double scale = 230.0;
	vector<double> endgrid, divgrid, l1, v1, v2;
	endgrid.push_back(0);
	endgrid.push_back(960);
	divgrid.push_back(1533);
	divgrid.push_back(-264.147);
	l1.push_back(960);
	l1.push_back(-0.79853);
	v1.push_back(1633);
	v1.push_back(-344);
	v2.push_back(12099);
	v2.push_back(300);

	return rw_dist(p1, p2, endgrid, divgrid, l1, v1, v2, scale);
}