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
#include <ctime>
#include <thread>
easywsclient::WebSocket::pointer ClientSender::ws = NULL;
easywsclient::WebSocket::pointer ClientSender::jsonClient = NULL;
//bool ClientSender::sendData = false;
bool jsonBreaker = false;
void ClientSender::SendJsonData()
{
    auto currentTime = std::chrono::system_clock::now();
    
    while(true)
    {
        
        if(!jsonClient || jsonClient->getReadyState()==WebSocket::CLOSED)
        {
            jsonClient = WebSocket::from_url("ws://localhost:8225/sender");
        }
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsedtime = end - currentTime;
        if (elapsedtime.count()*1000 > 60000)
        {
            jsonClient->send("day la json");
            while(true)
            {
                jsonClient->poll(-1);
                ws->dispatch([](const std::string& message){
                    cout<<"from client: "<<message<<endl;
                    if(message == "ok")
                    {
                        //                    ws->close();
                        jsonBreaker = true;
                        jsonClient->close();
                    }
                });
                
                
            }
        }
        if(jsonBreaker)
           break;
    }
}

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
        ws->sendBinary(buffer);
        
//        cout<<"send hello\n";
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
    buffer.clear();
    return false;
}
//convert cv::Mat to base64
//return true if base64 data not empty
//return false all other;
bool ClientSender::Mat2Base64(cv::Mat source){
    //variable array hold image data compress from cv::Mat
    
    base64Data = "";
    cv::imencode(".png",source,buffer);
    uchar *msgBase64 = new uchar[buffer.size()];
    for(int i=0; i<buffer.size();i++)
    {
        msgBase64[i] = buffer[i];
    }
    base64Data = base64_encode(msgBase64, buffer.size());
    
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

