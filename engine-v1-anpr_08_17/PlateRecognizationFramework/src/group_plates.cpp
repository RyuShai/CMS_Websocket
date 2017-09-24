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
#include <map>
#include "group_plates.h"
#include "r_distance.cpp"
#include "trackingkalman.h"
#include "tracking_thanhnn.cpp"


using namespace cv;
using namespace std;

struct grouping_data
{
    vector <vehicle> Queue;
    int IDs;
    int numPlates;
    bool pushed_to_cnn;
};
map <int,int> location_grouping;
//int location_grouping[100000];
vector <grouping_data> grouping;

//const int Range_max_frame_each_plate = 30;
const float time_one_frame = 0.00001111;//(1s / 25 hinh) (tinh theo h)
bool firstpush=true;

result_group group_plates(vector <Rect> plates_square, vector <vehicle> plates_process, Mat framefull, Rect cropRect,int frameID,int mode)
{
    if (firstpush)
    {
        grouping_data firstdata;
        grouping.push_back(firstdata);
        firstpush=false;
    }
    ObjectUtils objects;
    //init vector Point2d: centers
    result_group result;
    vector<Point2d> centers;
    Point2d point;
    for (int i = 0; i < (int)plates_square.size(); i++)
    {
        point.x = plates_square[i].x;//+plates_square[i].width/2+cropRect.x;
        point.y = plates_square[i].y;//+plates_square[i].height/2+cropRect.y;
        
        centers.push_back(point);
    }
    
    //ToDO: Kalman tracking
    //Struct_kalmans resultKM = process_grouping(framefull,centers);
    //ToDO: Thanhnn tracking
    Struct_kalmans resultKM = group_plate_thanhnn(framefull,centers,frameID,mode);

    //ToDO: grouping + cal speed
    
    if (resultKM.centers_out.size()>0)
        for (int i=0; i< resultKM.centers_out.size();i++)
        {
            //cout <<resultKM.IDs[i]<<"\t"<< location_grouping[resultKM.IDs[i]]<<"\t";
            if (location_grouping[resultKM.IDs[i]]==0) //chua ton tai queue chua IDs nay -> tao queue moi chua data
            {
                bool pushed=false;
                plates_process[i].vehicleImage = framefull.clone();
                //create grouping_data: data
                grouping_data data;
                data.Queue.push_back(plates_process[i]);
                data.IDs = resultKM.IDs[i];
                data.pushed_to_cnn = false;
                if (plates_process[i].plate_croped_detail.isplate)
                    data.numPlates = 1;
                else data.numPlates = 0;

                for (int j=1;j<grouping.size();j++)
                    if (grouping[j].IDs ==0 && pushed==false)
                    {
                        grouping[j] = data;
                        location_grouping[resultKM.IDs[i]] = j;
                        pushed=true;
                    }
                if (pushed==false)
                {
                    grouping.push_back(data);
                    location_grouping[resultKM.IDs[i]] = grouping.size()-1;
                }
                
                result.speed_plates.push_back("0");
                //result.speed_plates.push_back(objects.FloatToStr(resultKM.IDs[i]));
                result.rect_plates.push_back(resultKM.centers_out[i]);
            }
            else //da ton tai queue chua IDs nay, push data vao queue da co 
            {
                int location_push = location_grouping[resultKM.IDs[i]];
                grouping[location_push].Queue.push_back(plates_process[i]);
                if (plates_process[i].plate_croped_detail.isplate)
                    grouping[location_push].numPlates++;
                //ToDO: Free Mat if numPlates > MaxPlateProcessCNN
                if (grouping[location_push].numPlates > MaxPlateProcessCNN)
                    grouping[location_push].Queue[grouping[location_push].Queue.size()-1].plate_croped_detail.plate_extended.release();
                //ToDo: cal speed_plates -----------------------------------------------------
                float speed;
                if (grouping[location_push].Queue.size() == MaxFrameForEstimateSpeed)
                {
                    vehicle a = plates_process[i];
                    vehicle b = grouping[location_push].Queue[0];
                    float distance_frame = a.frameID-b.frameID;
                    float distance = d_dist((float)a.locationX+cropRect.x+a.plate_croped_detail.cn1.x,(float)a.locationY+cropRect.y+a.plate_croped_detail.cn1.y,(float)b.locationX+cropRect.x+b.plate_croped_detail.cn1.x,(float)b.locationY+cropRect.y+b.plate_croped_detail.cn1.y);
                    //float distance = 1;
                    speed = distance/1000/((float)distance_frame*(float)time_one_frame);
                }
                else 
                    speed = grouping[location_push].Queue[grouping[location_push].Queue.size()-2].speed;

                grouping[location_push].Queue[grouping[location_push].Queue.size()-1].speed = speed;
                speed = ceilf(speed);
                result.speed_plates.push_back(objects.FloatToStr(speed));
                //result.speed_plates.push_back(objects.FloatToStr(grouping[location_push].IDs));
                result.rect_plates.push_back(resultKM.centers_out[i]);
            }
            
        }
    
    //ToDO: chose group for cnn + free group
    for (int i=1;i<grouping.size();i++)
    {
        //cout << grouping[i].numPlates <<endl;
        //chose group to cnn
        if (grouping[i].numPlates == MaxPlateProcessCNN && grouping[i].pushed_to_cnn == false)
        {
            queue <vehicle> queuevehicle;
            for (int j=0;j<grouping[i].Queue.size();j++)
                if (grouping[i].Queue[j].plate_croped_detail.isplate)
                    queuevehicle.push(grouping[i].Queue[j]);

            queuevehicle.front().vehicleImage = grouping[i].Queue[0].vehicleImage.clone();
            queuevehicle.front().square_cascade = grouping[i].Queue[0].square_cascade;

            result.queuepush.push_back(queuevehicle);
            grouping[i].pushed_to_cnn = true;
        }
        //free group
        if (frameID - grouping[i].Queue[grouping[i].Queue.size()-1].frameID >= Range_max_frame_each_plate)
        {
            //group chua push -> push to cnn
            if (grouping[i].numPlates < MaxPlateProcessCNN && grouping[i].numPlates>0 && grouping[i].pushed_to_cnn ==false)
            {
                queue <vehicle> queuevehicle;
                for (int j=0;j<grouping[i].Queue.size();j++)
                    if (grouping[i].Queue[j].plate_croped_detail.isplate)
                        queuevehicle.push(grouping[i].Queue[j]);

                queuevehicle.front().vehicleImage = grouping[i].Queue[0].vehicleImage.clone();
                queuevehicle.front().square_cascade = grouping[i].Queue[0].square_cascade;

                result.queuepush.push_back(queuevehicle);
                grouping[i].pushed_to_cnn = true;
            }   
            //clear group
            location_grouping[grouping[i].IDs]=0;
            grouping[i].IDs = 0;
            grouping[i].numPlates=0;
            grouping[i].pushed_to_cnn = false;

            for (int j=0;j<grouping[i].Queue.size();j++)
            {
                //Dear Bach, anh push ra 4 point o day nhe:
                /*Point lefttop_square, cornerplate1,cornerplate2,cornerplate3,cornerplate4;
                vehicle a = grouping[i].Queue[j];
                lefttop_square.x = a.locationX - a.plate_croped_detail.cn1.x + cropRect.x;
                lefttop_square.y = a.locationY - a.plate_croped_detail.cn1.y + cropRect.y;
                cornerplate1 = lefttop_square+a.plate_croped_detail.cn1;
                cornerplate2 = lefttop_square+a.plate_croped_detail.cn2;
                cornerplate3 = lefttop_square+a.plate_croped_detail.cn3;
                cornerplate4 = lefttop_square+a.plate_croped_detail.cn4;
                cout << a.frameID << "\t" << cornerplate1 <<"\t"<< cornerplate2 <<"\t"<< cornerplate3 <<"\t"<< cornerplate4 <<"\t"<<endl;*/

            }
            grouping[i].Queue.clear();
        }
    }
    
    //ToDO: Push kết quả nội suy tới queue vẽ hình
    if (resultKM.assigned_traces.size()>0)
        for (int i=0; i< resultKM.assigned_traces.size();i++)
        {
            float speed=0;
            for (int j=1;j<grouping.size();j++)
                if (grouping[j].IDs == resultKM.track_id[i])
                {
                    speed = grouping[j].Queue[grouping[j].Queue.size()-1].speed;
                    break;
                }
            //result.speed_plates.push_back(objects.FloatToStr(resultKM.track_id[i]));
            speed = ceilf(speed);
            result.speed_plates.push_back(objects.FloatToStr(speed));
            result.rect_plates.push_back(resultKM.assigned_traces[i]);
        }

    return result;
}
