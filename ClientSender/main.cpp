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
using namespace std;
using namespace cv;
/*
 * 
 */

int main(int argc, char** argv) {
    cout<<"start"<<endl;
    Mat img = imread("/home/shai/Pictures/animation.png");
    ClientSender sender;
    sender.setWsUrl("ws://localhost:8224/sender");
    sender.Mat2Base64(img);
    sender.Connect2Server();
//    sender.SendImage2Server(img);
    cout<<"end"<<endl;
//    sender.Mat2Base64(img);
    return 0;
}

