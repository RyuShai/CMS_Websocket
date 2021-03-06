/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ClientSender.cpp
 * Author: shai
 * 
 * Created on September 20, 2017, 11:14 AM
 */

#include "ClientSender.h"
 easywsclient::WebSocket::pointer ClientSender::ws = NULL;
//bool ClientSender::sendData = false;
bool sendData = true;
ClientSender::ClientSender() {
    //        ws = WebSocket::from_url(wsUrl);
    //    std::cout<<"init sender "<<ws->getReadyState()<<std::endl;
    
}

ClientSender::ClientSender(const ClientSender& orig) {
}

ClientSender::~ClientSender() {
}

void ClientSender::setWsUrl(std::string wsUrl) {
    this->wsUrl = wsUrl;
}

std::string ClientSender::getWsUrl() const {
    return wsUrl;
}

//handle text message receive from server
void ClientSender::HandleMessage(const std::string& message){
    //    std::cout<<"Server: "<<message<<std::endl;
    std::cout<<"from server: "<<message<<std::endl;
    if(message == "getVideo")
    {
        ws->send("ahihi");
    }
}

bool ClientSender::SendImage2Server(){
    if(ws->getReadyState() == WebSocket::CLOSED)
    {
        ws = WebSocket::from_url(wsUrl);
    }
        if(!base64Data.empty())
        {
            ws->send(base64Data);
            cout<<"send hello\n";
            sendData = false;
        }
        else
        {
            cout<<"empty data\n";
        }
        while(ws->getReadyState() != WebSocket::CLOSED)
        {
            //wait until connected
            ws->poll(-1);
            ws->dispatch([](const std::string& message){
                cout<<"from client: "<<message<<endl;
                if(message == "ok")
                {
//                    ws->close();
                    sendData = true;
                }
                else if(message == "stop")
                {
                    sendData = false;
                    ws->close();
                }
            });
                      
//            ws->close();
            if(sendData)
            {
                return true;
            }        
        }
        return false;
}
//convert cv::Mat to base64
//return true if base64 data not empty
//return false all other;
bool ClientSender::Mat2Base64(cv::Mat source){
    //variable array hold image data compress from cv::Mat
    std::vector<uchar> buffer;
    base64Data = "";
    cv::imencode(".png",source,buffer);
    uchar *msgBase64 = new uchar[buffer.size()];
    for(int i=0; i<buffer.size();i++)
    {
        msgBase64[i] = buffer[i];
    }
    base64Data = base64_encode(msgBase64, buffer.size());
    buffer.clear();
    delete msgBase64 ;
    return base64Data == "" ? false : true;
}

bool ClientSender::Connect2Server(){
    if(!wsUrl.empty())
    {
        std::cout<<"server connecting ..."<<std::endl;
        ws = WebSocket::from_url(wsUrl);
        while(ws->getReadyState() != WebSocket::CLOSED)
        {
            //wait until connected
//            while(true)
//            {
                ws->poll(-1);
                ws->dispatch([](const std::string& message){
                cout<<"from server: "<<message<<endl;
                if(message == "getVideo")
                {
                    cout<<"get video\n";
                    ws->close();
                }
            });
//            if(sendData)
//                break;
//            }
            
        }
        cout<<"go out\n";
        return true;
    }
    else
    {
        std::cout<<"connected failed - url not set yet"<<std::endl;
        return false;
    }
}


//disconnect from server 
bool ClientSender::DisconnectFromServer(){
    if(ws)
    {
        if(ws->getReadyState()!=WebSocket::CLOSED)
        {
            ws->close();
            std::cout<<"disconnected"<<std::endl;
            return true;
        }
    }
    else
    {
        return false;
    }
}

int ClientSender::getWSState(){
    return ws->getReadyState();
}

