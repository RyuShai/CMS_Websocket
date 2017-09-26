#include "iostream"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <fstream>
#include <ctime>
#include <thread>
#include <chrono>
#include <cstring>
#include <string>
#include <limits>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "PlateRecognizator.h"
#include "PlateReader.h"
#include "crop_plate_corner.h"
#include "group_plates.h"
#include "estimate_square_cascade.h"



using namespace pr;
using namespace std;
using namespace cv;

long num_plate_detected[MaxKindsPlate];
const long num_plate_estimate_square_cascade = 1000;
queue <long> car_estimate_square_cascade;

float Neighbor[2];
PlateReader::PlateReader(cv::Rect cropRect,
                         std::queue<FrameData> &frameQueue_, std::mutex &frameMutex_,
                         QueuePlates &plateQueue_,
                         std::queue<PlateData> &plateQueue_showFrame_,std::mutex &plateQueue_showFrameMutex_)
    : frameQueue(frameQueue_), frameMutex(frameMutex_),
      plateQueue(plateQueue_),
      plateQueue_showFrame(plateQueue_showFrame_),
      plateQueue_showFrameMutex(plateQueue_showFrameMutex_)

{

    this->cropRect = cropRect;
    std::string queuesize = pr::IOData::GetCongfigData("frameSize:");
    this->QueueSize = atoi(queuesize.c_str());
    //Square = 0
    //Long = 1
    int MinWidth[2], MinHeight[2], MaxWidth[2], MaxHeight[2];
    float DetectScale[2];

    MinWidth[0] = atoi(IOData::GetCongfigData("SquareMinWidth:").c_str());
    MinHeight[0] = atoi(IOData::GetCongfigData("SquareMinHeight:").c_str());
    MaxWidth[0] = atoi(IOData::GetCongfigData("SquareMaxWidth:").c_str());
    MaxHeight[0] = atoi(IOData::GetCongfigData("SquareMaxHeight:").c_str());

    MinWidth[1] = atoi(IOData::GetCongfigData("LongMinWidth:").c_str());
    MinHeight[1] = atoi(IOData::GetCongfigData("LongMinHeight:").c_str());
    MaxWidth[1] = atoi(IOData::GetCongfigData("LongMaxWidth:").c_str());
    MaxHeight[1] = atoi(IOData::GetCongfigData("LongMaxHeight:").c_str());

    DetectScale[0] = atof(IOData::GetCongfigData("SquareDetectScale:").c_str());
    DetectScale[1] = atof(IOData::GetCongfigData("LongDetectScale:").c_str());
    Neighbor[0] = atof(IOData::GetCongfigData("SquareNeighbor:").c_str());
    Neighbor[1] = atof(IOData::GetCongfigData("LongNeighbor:").c_str());

    for (int mode=0;mode<MaxKindsPlate;mode++)
    {
        //square
        string kindcascade = "car_cascade:";
        int kindsizecascade = 0;
        if (mode==0)
        {
            kindcascade = "car_cascade:";
            kindsizecascade = 0;
        
        }
        if (mode==1)
        {
            kindcascade = "car_horizontal_cascade:";
            kindsizecascade = 1;
        }
        recognizator_vehicle[mode].Init(IOData::GetCongfigData(kindcascade),
                cv::Size(MinWidth[kindsizecascade], MinHeight[kindsizecascade]), 
                cv::Size(MaxWidth[kindsizecascade], MaxHeight[kindsizecascade]), 
                DetectScale[kindsizecascade], Neighbor[kindsizecascade]);

    }
    for (int i=0;i<MaxKindsPlate;i++)
        num_plate_detected[i]=0;

    
}

void PlateReader::operator()()
{
    while (true)
    {
        this_thread::sleep_for(chrono::milliseconds(1));

        //TODO: sonhh check frameQueue
        //cout << " Frame: "<<frameQueue.size()<<endl;

        if (frameQueue.empty())
            continue;
            
        frameMutex.lock();
        FrameData thisFrame = frameQueue.front();
        thisFrame.framefull=thisFrame.frame.clone();
        frameQueue.pop();

        frameMutex.unlock();
        //ryu
        // cout<<"Ryu 1 "<<frameQueue.size() <<" "<<this->QueueSize<<endl;
        //shai
        if (int(frameQueue.size()) > this->QueueSize)
            continue;

        cv::Mat frameROI = thisFrame.frame(cropRect);
        std::vector<pr::PlateRegion> plates_square[MaxKindsPlate];// car_plates_square, car_horizontal_plates_square;
        std::vector<string> vehicle_speed[MaxKindsPlate];// car_speed,car_horizontal_speed;

        for (int mode=0; mode<MaxKindsPlate; mode++)//Duyet tat ca kieu bien xe co the co
        {
            std::thread PlateReaderThread([this, &frameROI, &plates_square, &vehicle_speed, &thisFrame, mode] {

                recognizator_vehicle[mode].SetImg(frameROI);
                plates_square[mode] = recognizator_vehicle[mode].GetPlateRegions();
                
                //ToDO: Estimate square cascade; ReInit recognizator_vehicle cascade
                for (int i = 0; i < (int)plates_square[mode].size(); i++)
                {
                    num_plate_detected[mode]++;
                    //cout << "num: " << num_plate_detected[mode]<<endl;
                    /*
                    if (num_plate_detected[mode]%50000 <= num_plate_estimate_square_cascade) //Each 50.000 plate recalculate plate size for cascade
                    {
                        long multiply = plates_square[mode][i].region.width*plates_square[mode][i].region.height;
                        car_estimate_square_cascade.push(multiply);
                        if (num_plate_detected[mode]%50000 == num_plate_estimate_square_cascade)
                        {
                            //ToDO: Estimate square cascade
                            cascade_input cascade_square_result = estimate_square_cascade(car_estimate_square_cascade);
                            //ToDO: ReInit recognizator_vehicle cascade
                            string kindcascade = "car_cascade:";
                            int kindsizecascade = 0;
                            if (mode==1)
                            {
                                kindcascade = "car_horizontal_cascade:";
                                kindsizecascade = 1;
                            }
                            recognizator_vehicle[mode].Init(IOData::GetCongfigData(kindcascade),
                                cv::Size(cascade_square_result.carSquareMin_x, cascade_square_result.carSquareMin_y), 
                                cv::Size(cascade_square_result.carSquareMax_x, cascade_square_result.carSquareMax_y), 
                                cascade_square_result.detectScale, Neighbor[kindsizecascade]);
                            //ToDO: Free queue
                            while (!car_estimate_square_cascade.empty())
                                car_estimate_square_cascade.pop();
                        }
                    }
                    */
                }  

                // ToDO: init vector Plates: plates_process
                vector <vehicle> plates_process;
                for (int i = 0; i < (int)plates_square[mode].size(); i++)
                {
                    cv::Rect RegionPlus = cv::Rect(
                        std::max(plates_square[mode][i].region.x - 15, 0),
                        std::max(plates_square[mode][i].region.y - 15, 0),
                        std::min(plates_square[mode][i].region.width + 30, frameROI.cols - plates_square[mode][i].region.x + 15-1),
                        std::min(plates_square[mode][i].region.height + 30, frameROI.rows - plates_square[mode][i].region.y + 15-1));
                    
                    vehicle a;
                    a.locationX = plates_square[mode][i].region.x;
                    a.locationY = plates_square[mode][i].region.y;    
                    a.time = objects.convert_time(thisFrame.frameTime);
                    a.frameID = thisFrame.frameID;
                    a.plate = frameROI(RegionPlus);
                    a.square_cascade = plates_square[mode][i].region;
                    a.CurrentDateTime = thisFrame.frameTime;
                    a.plate_croped_detail = crop_plate_corner(frameROI(RegionPlus),mode); //crop plate
                    plates_process.push_back(a);
                     //ryu
                     if(plates_process.size()>100)
                     {
                        plates_process.pop_back();
                     }
                     cout<<"RYU plates_process"<<plates_process.size()<<endl;
                     //shai
                }
                //ToDO: init vector plates_square
                vector <Rect> plates_square_region;
                for (int i = 0; i < (int)plates_square[mode].size(); i++)
                {
                    plates_square_region.push_back(plates_square[mode][i].region);
                    //ryu
                    if(plates_square_region.size()>100)
                    {
                        plates_square_region.pop_back();
                    }
                    cout<<"RYU plates_square_region"<<plates_square_region.size()<<endl;
                    //shai
                }
                    

                // //ToDO: kalman tracking & group_plates
                //input: vector rect: plates_square[mode]; vector vehicle: plates_process; frameFull
                //output: vector Rect + vector Speed; group result: input for cnn
                result_group result = group_plates(plates_square_region, plates_process, thisFrame.frame, cropRect,thisFrame.frameID,mode);
                //result.rect_plates = plates_square_region;
                for (int i=0; i< result.speed_plates.size();i++)
                    if (result.speed_plates[i]!="0" & result.speed_plates[i]!="1")
                    {
                        putText(thisFrame.frame, result.speed_plates[i]+"km/h", cvPoint(result.rect_plates[i].x+cropRect.x,result.rect_plates[i].y+cropRect.y), FONT_HERSHEY_COMPLEX_SMALL, 2, cvScalar(0, 0, 100), 20, CV_AA);
                        putText(thisFrame.frame, result.speed_plates[i]+"km/h", cvPoint(result.rect_plates[i].x+cropRect.x,result.rect_plates[i].y+cropRect.y), FONT_HERSHEY_COMPLEX_SMALL, 2, cvScalar(255, 255, 0), 2, CV_AA);
                    }
                //ToDO: Push data to CNN
                for (int i=0;i<result.queuepush.size();i++)
                {
                    plateQueue.platequeue[mode].push(result.queuepush[i]);
                    //ryu
                    if(plateQueue.platequeue[mode].size()>100)
                    {
                        plateQueue.platequeue[mode].pop();
                    }
                    cout<<"RYU plateQueue.platequeue[mode].size()"<<plateQueue.platequeue[mode].size()<<endl;
                    //shai
                }
                
                //ryu
                recognizator_vehicle[mode].ClearImg();
                for(int i=0; i<plates_square[mode].size();i++)
                {
                     plates_square[mode].pop_back();
                }
                plates_square[mode].clear();
                //shai
            });
            PlateReaderThread.join();
        }
            
        //Push square plate to queue for video realtime
        PlateData data;
        data.frame = thisFrame.frame;
        data.frameID = thisFrame.frameID;
        for (int mode=0; mode<MaxKindsPlate; mode++)
        {
            data.PlatesSquare[mode] = plates_square[mode];
            //data.vehicle_speed[mode] = vehicle_speed[mode];
        }

        plateQueue_showFrame.push(data);
        //ryu
        if(plateQueue_showFrame.size()>100)
        {
            plateQueue_showFrame.pop();
        }
        // cout<<"RYU plateQueue_showFrame"<<plateQueue_showFrame.size()<<endl;
        //shai
    }
}
