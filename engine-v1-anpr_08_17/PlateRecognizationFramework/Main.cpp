#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <fstream>
#include <ctime>
#include <queue>
#include <thread>
#include <mutex>
#include <exception>

#include "VideoReader.h"
#include "PlateReader.h"
#include "PlateRecognizator.h"
#include "PlateRecognitionAction.h"

#include <sys/socket.h> 
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h> 

#include "easywsclient.cpp"
#include "base64.cpp"

#include <json/json.h>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/value.h>

#include "ClientSender.h"

using namespace pr;
using namespace cv;
// using namespace std;

queue<FrameData> frameQueue;//Frames from Video Streaming
mutex frameQueueMutex;

//Mode: kinds plate
//0: While square
//1: While long
//2: Blue long
//3: Red long
//4: Blue square
//5: Red square
cv::Rect cropRect;

QueuePlates plateQueue;

queue<PlateData> plateQueue_showFrame;
mutex plateQueue_showFrameMutex;

pr::ObjectUtils objects;



string engine_ip = "10.12.11.198";

bool status = true;

string json_decode(string json, string keyW){
		json_object *jobj = json_tokener_parse(json.c_str());
		json_object_object_foreach(jobj, key, val)
    	{
			string str = string(key);
			if(str==keyW)
			   return json_object_get_string(val);
		}
		return 0; 
}	
//--------------
using easywsclient::WebSocket;
static WebSocket::pointer webClient = NULL ;
static Websocket::pointer jsonWebClient =NULL;
 bool isBreak = false;
//--------------
void showFrame()
{
	// cout<<"RYU FUCK"<<endl;
	int nframe = 0;
	auto start = CLOCK_NOW();
	int fps = 26;
	int delay = 1000/fps;
	while (true)
	{
		auto startime = CLOCK_NOW();
		//this_thread::sleep_for(chrono::milliseconds(5));
		if (plateQueue_showFrame.empty())
			continue;
		//cout << plateQueue_showFrame.size()<<endl;
		plateQueue_showFrameMutex.lock();
		PlateData data = plateQueue_showFrame.front();
		plateQueue_showFrame.pop();
		plateQueue_showFrameMutex.unlock();

		cv::Mat frame = data.frame.clone();
		cv::rectangle(frame, cropRect, cv::Scalar(0, 255, 0), 2, 8, 0);
		for (int mode = 0; mode <MaxKindsPlate; mode++) 
			for (int i = 0; i < (int)data.PlatesSquare[mode].size(); i++)
			{
				data.PlatesSquare[mode][i].region.x +=cropRect.x;
				data.PlatesSquare[mode][i].region.y +=cropRect.y;
				//if (objects.isplate(frame(data.PlatesSquare[t][i].region)))
				{
					cv::rectangle(frame, data.PlatesSquare[mode][i].region, cv::Scalar(255, 0, 255), 2, 8, 0);
					/*if (data.vehicle_speed[mode][i] != "0")
					{
						putText(frame, data.vehicle_speed[mode][i] + "km/h", cvPoint(data.PlatesSquare[mode][i].region.x, data.PlatesSquare[mode][i].region.y), FONT_HERSHEY_COMPLEX_SMALL, 2, cvScalar(0, 0, 0), 20, CV_AA);
						putText(frame, data.vehicle_speed[mode][i] + "km/h", cvPoint(data.PlatesSquare[mode][i].region.x, data.PlatesSquare[mode][i].region.y), FONT_HERSHEY_COMPLEX_SMALL, 2, cvScalar(255, 255, 255), 2, CV_AA);
					}*/
				}
			}
		cv::resize(frame, frame, cv::Size(), 0.5, 0.5);

		vector<uchar> buf;
		imencode(".jpg", frame, buf);  
		uchar *enc_msg = new uchar[buf.size()];
		for (int i = 0; i < buf.size(); i++)
			enc_msg[i] = buf[i];
		string encodedFrame = base64_encode(enc_msg, buf.size());
		buf.clear();
		
		delete[] enc_msg;

		using easywsclient::WebSocket;

		string json = "{\"type\":\"engine\",\"data\":\"" + encodedFrame + "\",\"engine_id\":\"" + engine_ip + "\"}";

		std::unique_ptr<WebSocket> ws(WebSocket::from_url("ws://10.12.11.93:8126/foo"));
		
		cout<<"F YEAH\n";
		if(webClient==NULL || webClient->getReadyState()==WebSocket::CLOSED)
		{
			cout<<"RYU int connect server\n";	
			webClient = WebSocket::from_url("ws://localhost:8224/sender");
		}
		cout<<"RYU here3\n";
		// assert(ws);
		if (status) {
			webClient->send(encodedFrame);
			isBreak = false;
			
		}
		while (webClient->getReadyState() != WebSocket::CLOSED) {
			// WebSocket::pointer wsp = &*webClientws; // <— because a unique_ptr cannot be copied into a lambda
			webClient->poll(-1);
			webClient->dispatch([](const std::string & message) {
				// cout<<message.c_str()<<endl;
				// string current_engine = json_decode(message.c_str(), "engine_id");
				// if (current_engine == engine_ip) {
				// 	status = true;
					
				// } else {
				// 	status = false;
					
				// }
				if(message == "ok")
					isBreak=true;
				// wsp->close();
				// delete *wsp;
			});
			if(isBreak)
			{
				break;
			}
		}
		cv::imwrite("/var/www/html/web-client/web/video_not_found.jpg", frame);
		//  cv::imshow("Plate1", frame);
		// //this_thread::sleep_for(chrono::milliseconds(15));

		// imshow("tst",frame);
		cv::waitKey(1);


		nframe++;
		auto end = CLOCK_NOW();
		ElapsedTime elapsed = end - start;
		if (nframe % 100 == 0)
		{
			cout << " display fps=" << nframe / elapsed.count() << endl;
		}
		while (1)
		{
			auto endtime = CLOCK_NOW();
			ElapsedTime elapsedtime = endtime - startime;
			//cout << elapsedtime.count()*1000 << endl;
			if (elapsedtime.count()*1000 < delay)
				this_thread::sleep_for(chrono::milliseconds(1));
			else break;
		}
		this_thread::sleep_for(chrono::milliseconds(100));
	}
}


int main()
{


	int cropX = atoi(IOData::GetCongfigData("cropX:").c_str());
	int cropY = atoi(IOData::GetCongfigData("cropY:").c_str());
	int cropWidth = atoi(IOData::GetCongfigData("cropWidth:").c_str());
	int cropHeight = atoi(IOData::GetCongfigData("cropHeight:").c_str());
	cropRect = cv::Rect(cropX, cropY, cropWidth, cropHeight);

	VideoReader VideoReader(IOData::GetLinkURL(),
							atoi(IOData::GetCongfigData("video_width:").c_str()),
							atoi(IOData::GetCongfigData("video_height:").c_str()),
							cropRect,
							frameQueue);

	PlateReader plateReader(
		cropRect,
		frameQueue,
		frameQueueMutex,
		plateQueue,
		plateQueue_showFrame,
		plateQueue_showFrameMutex);

	PlateRecognitionAction PlateRecognitionAction(
			cropRect,
			plateQueue);

	std::vector<std::thread> ths;
	ths.push_back(std::thread(VideoReader));
	ths.push_back(std::thread(plateReader));
	ths.push_back(std::thread(PlateRecognitionAction));
	ths.push_back(std::thread(showFrame));



	for (auto &t : ths)
		t.join();
	
}
