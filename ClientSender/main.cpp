/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: shai
 *
 * Created on September 20, 2017, 11:08 AM
 */

#include <cstdlib>
#include "ClientSender.h"
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <thread>
using namespace std;
using namespace cv;
/*
 * 
 */
void doSomething()
{
    cout<<"do something "<<this_thread::get_id()<<endl;
    this_thread::sleep_for(std::chrono::milliseconds(500));
    
}
int main(int argc, char** argv) {
    cout<<"start"<<endl;
//    cv::VideoCapture cam("/home/shai/Downloads/Telegram Desktop/test.mp4");
    cv::VideoCapture cam("http://07c2.vp9.tv:3395/chn/NEM2/v.m3u8");
//        Mat img = imread("/home/shai/Pictures/animation.png");
    Mat frame;
    ClientSender sender;
    sender.setWsUrl("ws://localhost:8224/sender");
    //    std::thread listen(sender.Connect2Server);
    //    listen.join();
    while(true){
        sender.Connect2Server();
        while(cam.isOpened() )
        {
            
            cout<<"while cam opend"<<this_thread::get_id()<<endl;
//            std::this_thread::sleep_for(chrono::seconds(5));
            //        std::this_thread::yield();
            
                    cam>>frame;
                    resize(frame,frame, Size(frame.cols*0.5,frame.rows*0.5));
//                    imshow("test",frame);
//                    waitKey(40);
                    if(sender.Mat2Base64(frame))
                    {
                        sender.SendImage2Server();
                    }
                    waitKey(30);
                    cout<<"sendata: "<<sendData<<endl;
                    if(!sendData)
                    {
                        cout<<"break\n";
                        break;
                    }
//                    std::this_thread::sleep_for(chrono::milliseconds(40));
//                    if(sender.getWSState() == WebSocket::CLOSED)
//                    {
//                        cam.release();
//                    }
        }
    }
    
    
    
    //    sender.Mat2Base64(img);
    //    sender.Connect2Server();
    //    sender.SendImage2Server(img);
    cout<<"end"<<endl;
    //    sender.Mat2Base64(img);
    return 0;
}

