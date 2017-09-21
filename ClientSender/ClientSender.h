/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ClientSender.h
 * Author: shai
 *
 * Created on September 20, 2017, 11:14 AM
 */

#ifndef CLIENTSENDER_H
#define CLIENTSENDER_H
#include "easywsclient.hpp"
#include <stdio.h>
#include <string>
#include <iostream>
#include <assert.h>
#include <opencv2/opencv.hpp>
#include "base64.h"

using namespace cv;
using easywsclient::WebSocket;
using namespace std;
        
extern bool sendData;
class ClientSender {
public:
    ClientSender();
    ClientSender(const ClientSender& orig);
    virtual ~ClientSender();
    void setWsUrl(std::string wsUrl);
    std::string getWsUrl() const;
    bool Connect2Server();
    bool DisconnectFromServer();
    bool SendImage2Server();
    std::string closeCode="";
    bool Mat2Base64(cv::Mat source);
    int getWSState();
    
private:
    //variable
    //variable connect to websocket server
    static WebSocket::pointer ws ; 
    //variable url to connect
    std::string wsUrl;
    //variable array hold image data compress from cv::Mat
    std::vector<uchar> buffer;
    //variable base64 use to send to server
    std::string base64Data;
    //variable emit when starting send data
    
    
    //function
    void HandleMessage(const std::string &message);
    
    
    
};

#endif /* CLIENTSENDER_H */

