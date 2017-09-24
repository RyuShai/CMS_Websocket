#include <queue>
#include <stack>
#include <mutex>
#include <chrono>
#include <string>
#include <thread>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "VideoReader.h"

using namespace pr;
using namespace std;
using namespace cv;

VideoReader::VideoReader(
    string url, 
    int width, 
    int height,
    Rect cropRect, 
    queue<FrameData> &queue_)
    : frameQueue(queue_)
{
    this->url = url;
    this->width = width;
    this->height = height;
}

void VideoReader::operator()()
{
    VideoCapture cap;
    while (true)
    {
        cap.open("/home/shai/Downloads/Telegram Desktop/test.mp4");
        //double fps = cap.get(CV_CAP_PROP_FPS);
        int fps = 25;
	    int delay = 1000/fps;
        if (cap.isOpened())
        {
            long long id = 0;
            while (1)
            {
                auto startime = CLOCK_NOW();
                id++;
                Mat frame;
                cap >> frame; 
                
                if (frame.data)
                {
                    resize(frame, frame, cv::Size(width, height));
                    FrameData thisFrame;
                    thisFrame.frame = frame;
                    thisFrame.frameID = id;
                    thisFrame.frameTime = objects.getCurrentDateTime();
                    
                    frameQueue.push(thisFrame);
                    if(frameQueue.size()>100)
                    {
                        frameQueue.pop();
                    }
                    // cout<<"Ryu queue frame size : "<<frameQueue.size()<<endl;
                }
                else
                    break;
                this_thread::sleep_for(chrono::milliseconds(10));
                
                
                while (1)
                {
                    auto endtime = CLOCK_NOW();
                    ElapsedTime elapsedtime = endtime - startime;
                    if (elapsedtime.count()*1000 < delay)
                        this_thread::sleep_for(chrono::milliseconds(2));
                    else break;
                }
            }
        }
        else
        {
            cout << "\nLost connect!";
            this_thread::sleep_for(chrono::milliseconds(5000));
        }
    }
}
