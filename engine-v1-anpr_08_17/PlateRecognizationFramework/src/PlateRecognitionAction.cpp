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
#include <mutex>
#include <sys/stat.h>

#include "PlateRecognizator.h"
#include "PlateRecognitionAction.h"
#include "crop_plate_corner.h"
#include "crop_character_rect.h"

using namespace pr;
using namespace std;
using namespace cv;

PlateRecognitionAction::PlateRecognitionAction(
    cv::Rect cropRect,
    QueuePlates &plateQueue_)
    :plateQueue(plateQueue_)
{
    this->cropRect = cropRect;
    //isolator = new CarTextIsolation();
    dp = new TextReader(IOData::GetCongfigData("filedump:"));
}

void PlateRecognitionAction::operator()()
{
    while (true)
    {
        
        this_thread::sleep_for(chrono::milliseconds(1));
        for (int mode=0; mode<MaxKindsPlate;mode++)
        {
            if (plateQueue.platequeue[mode].empty())
                continue;
            
            plateQueue.platequeueMutex[mode].lock();
            queue <vehicle> queueprocess = plateQueue.platequeue[mode].front();
            mutex queueprocessMutex;
            plateQueue.platequeue[mode].pop();
            plateQueue.platequeueMutex[mode].unlock();

            //continue;

            int num_plates = queueprocess.size();
            vehicle plateprocess[num_plates];
            int i=0;
            while (!queueprocess.empty())
            {
                queueprocessMutex.lock();
                plateprocess[i]=queueprocess.front();
                i++;
                queueprocess.pop();
                queueprocessMutex.unlock();
            }
            //Plate recognition cnn
            for (int k=0;k<num_plates;k++)
            {
                //cout << "aaaaaaaaaaaaaaaaa"<<endl;
                //plate_corner after_crop = crop_plate_corner(plateprocess[k].plate);

                Rect Rect_small_CropPlate = plateprocess[k].plate_croped_detail.plate_content;
                Mat CropPlate = plateprocess[k].plate_croped_detail.plate_extended.clone();

                Mat small_CropPlate = CropPlate(Rect_small_CropPlate);
                std::vector<cv::Rect> charRects;

                std::thread carPlateThread([this, &CropPlate, &charRects, &plateprocess,&small_CropPlate,k,mode] {
                    charRects = getCharacterRect_Square(small_CropPlate, mode);
                    
                    if (charRects.size()>=6)
                    {

                        int dis_small_CropPlate = (CropPlate.size().width-small_CropPlate.size().width)/2;
                        std::vector<cv::Rect> charRects_bu = charRects;
                        for (int i = 0; i < (int)charRects_bu.size(); i++)
                            if (!(charRects_bu[i].x==0 && charRects_bu[i].y==0 && charRects_bu[i].width==0 && charRects_bu[i].height==0))
                            {
                                charRects_bu[i].x +=dis_small_CropPlate;
                                charRects_bu[i].y +=dis_small_CropPlate;
                            }

                        //Chuyen doi charRects sang size anh to
                        for (int i = 0; i < (int)charRects.size(); i++)
                        if (!(charRects[i].x==0 && charRects[i].y==0 && charRects[i].width==0 && charRects[i].height==0))
                        {
                            charRects[i].x +=dis_small_CropPlate;
                            charRects[i].y +=dis_small_CropPlate;
                            charRects[i].x = max(charRects[i].x-4, 0);
                            charRects[i].y = max(charRects[i].y-7, 0);
                            charRects[i].width = min(charRects[i].width + 8, CropPlate.cols -charRects[i].x - 1);
                            charRects[i].height = min(charRects[i].height + 14, CropPlate.rows -charRects[i].y -1);
                        }

                        stringWithPro PlateStrWP = dp->GetPlateString(charRects, CropPlate);
                        //ve hinh vuong len plate
                        for (int i = 0; i < (int)charRects.size(); i++)
                        {
                            //cv::rectangle(plateprocess[k].plate_croped_detail.plate_extended, charRects_bu[i], cv::Scalar(0, 255, 0), 1, 8, 0);
                            //string link = "/home/thanhnn/Desktop/Result/GetData/Char/";
                            //std::string folder = link+PlateStrWP.PlateStr[i];
                            //mkdir(folder.c_str(), ACCESSPERMS);
                            //imwrite(folder+"/"+PlateStrWP.PlateStr+"_"+objects.FloatToStr(i)+".jpg",CropPlate(charRects[i]));
                        }
                        //----------------------------
                        plateprocess[k].vehiclePlate = PlateStrWP.PlateStr;
                        plateprocess[k].pro = PlateStrWP.pro;
                    }
                    else 
                    {
                        plateprocess[k].vehiclePlate = "";
                        plateprocess[k].pro = 0;
                    }
                });
                carPlateThread.join();
                
                //cout << plateprocess[k].vehiclePlate << "\t"<<plateprocess[k].pro << endl;
            }
            objects.printf_result(plateprocess,num_plates,cropRect);
            //cout << "------------------------------------------"<<endl;
            
        }
    }
}