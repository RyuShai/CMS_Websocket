
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <iterator>
#include <vector>
#include "Qtrackers.h"
#include "trackingkalman.h"

#include <fstream>

#include <string>
#include <cstring>


using namespace std;
using namespace cv;


float dt = 1;
float Accel_noise_mag = 15;
int dist_thres = 80;
int maximum_allowed_skipped_frames = 5;
int max_trace_length = 5;


int fontFace = FONT_HERSHEY_SCRIPT_SIMPLEX;
double fontScale = 2;
int thickness = 3; 

Scalar Colors[] = { Scalar(255, 0, 0), Scalar(0, 255, 0), Scalar(0, 0, 255), Scalar(255, 255, 0), Scalar(0, 255, 255), Scalar(255, 0, 255), Scalar(255, 127, 255), Scalar(127, 0, 255), Scalar(127, 0, 127) };

QTrackers trackers(dt, Accel_noise_mag, dist_thres, maximum_allowed_skipped_frames, max_trace_length); // create Kalman trackers 
 



Struct_kalmans process_grouping(const Mat m_cur_frame,vector<Point2d> centers)
{
    Struct_kalmans object_kalmans;
    //*******************************************************//
    // Multiple object Kalman tracking
    vector<int> assignment;
    string text;
    if (centers.size()>0)
    {
            assignment = trackers.Update(centers, Point2d(m_cur_frame.cols,m_cur_frame.rows));
            
            for (int i=0;i< assignment.size();i++){
                    int idx = assignment[i];
                    QTrack* curTrack1= trackers.tracks[i];
                    unsigned long id = curTrack1->track_id;
                    if (idx != -1)
                    {
//                       // cv::putText(m_cur_frame, text, centers[idx] , fontFace, fontScale, Colors[curTrack1->track_id % 9],2,8);
//                       // circle(m_cur_frame, centers[idx], 10, Scalar(0, 255, 0), 3, CV_AA);
                        text = std::to_string(id);
                        object_kalmans.IDs.push_back(id);
                        object_kalmans.centers_out.push_back(centers[idx]);
                    }
                   
//                cout << trackers.tracks.size() << "  " <<  assignment[i] << endl;    
            }

    }
    
    Point cent;
    // draw centers of moving objects
    for (int i = 0; i< trackers.tracks.size(); i++)
    {
        QTrack* curTrack= trackers.tracks[i];
            int traceNum = curTrack->trace.size();

            if (traceNum>5)
            {
                    // draw trackers
    //                for (int j = 0; j<curTrack->trace.size(); j++)
    //                {
                         //  line(cur_frame, curTrack->trace[j], curTrack->trace[j + 1], Colors[curTrack->track_id % 9], 1, CV_AA);
    //                    circle(m_cur_frame, curTrack->trace[j], 2, Colors[curTrack->track_id % 9], 2, CV_AA);
    //                }
                int lastPostion = curTrack->trace.size() -1;
                int id = curTrack->track_id;
                text = std::to_string(id);
                cent = curTrack->trace[lastPostion];
 //               cv::putText(m_cur_frame, text, cent , fontFace, fontScale, Colors[curTrack->track_id % 9],2,8);

                //object_kalmans.assigned_traces.push_back(curTrack->trace);
                //object_kalmans.track_id.push_back(curTrack->track_id);
            }
    }

    return object_kalmans;
}

/*int main(int, char**)
{

    //cout << "Could not read video file" << endl;
    string video_name = "RAD2.mp4";
    //string video_name = "simple.mp4";
    
   // string log_filename = "logplate.txt";
    
    std::ifstream plate_loc_file("logplate.txt");
    
    
    //Mat image;
    VideoCapture video(video_name);
     
    // Check video is open
    if(!video.isOpened())
    {
        cout << "Could not read video file" << endl;
        return 1;
    }
    
        // Read first frame. 
    Mat cur_frame;
    Mat bg_frame;
    video.read(bg_frame);
    int height = bg_frame.rows;
    int width = bg_frame.cols;
    
  //  imshow("Window", bg_frame);
  //  cv::waitKey(1000);

    // Kalman delaration
    Mat img(500, 500, CV_8UC3);
    KalmanFilter KF(2, 1, 0); // trans matrix 2x2 ; measurment 1 x 1;   control vector 0;
    Mat state(2, 1, CV_32F);  //(phi, delta_phi) 
    Mat processNoise(2, 1, CV_32F);
    Mat measurement = Mat::zeros(1, 1, CV_32F);
    char code = (char)-1;
    
    //vector<unsigned long> IDs;
    //vector<vector<Point2d>> assigned_traces;
   // vector<int> assignment;
    string text;
    
    long frame_ind = 0;
    int a =0, b , c ;
    Point point;
    
    while(video.read(cur_frame))
    {
        frame_ind ++;
        Mat cur_img;
        
        
    //    imshow("Window", cur_frame);
        
        vector<Point2d> centers;
        while(a <= frame_ind )
        {
            if( a == frame_ind )
            {   
              //  cout << frame_ind << " " << a << " " << b << " " << c << " " << endl;
                point.x = b + 30;
                point.y = c + 320;
                
             //   circle(cur_frame, point, 20, Scalar(0, 255, 0), 3, CV_AA);
                centers.push_back(point);
            }
            plate_loc_file >> a >> b >> c;
        }
        
        vector<Rect> rects;
        Rect cur_rect;
        
      //  cout << frame_ind << " " << centers.size() << " " << endl;
        Point2d cur_point;

        
        Struct_kalmans resultKM = process_grouping(cur_frame,centers);

        if (resultKM.centers_out.size()>0)
        {    
            for (int i=0; i< resultKM.centers_out.size();i++){
                 circle(cur_frame, resultKM.centers_out[i], 10, Scalar(0, 255, 0), 3, CV_AA);
                text = std::to_string(resultKM.IDs[i]);
                cv::putText(cur_frame, text, resultKM.centers_out[i] , fontFace, fontScale, Colors[resultKM.IDs[i] % 9],2,8);
            }
        }
        
        
            Point cent;
        // draw centers of moving objects
        for (int i = 0; i< resultKM.assigned_traces.size(); i++)
        {
            vector<Point2d> curTrack= resultKM.assigned_traces[i];

                int traceNum = curTrack.size();

                if (traceNum>5)
                {
                        // draw trackers
                        for (int j = 0; j< curTrack.size(); j++)
                        {
                             //  line(cur_frame, curTrack->trace[j], curTrack->trace[j + 1], Colors[curTrack->track_id % 9], 1, CV_AA);
                            circle(cur_frame, curTrack[j], 2, Colors[resultKM.track_id[i] % 9], 2, CV_AA);
                        }
                        int lastPostion = curTrack.size();
                        int id = i;
                        text = std::to_string(id);
                        cent = curTrack[lastPostion-1];
                        //cv::putText(cur_frame, text, cent , fontFace, fontScale, Colors[i % 9],2,8);
                }
        }

        imshow("Window", cur_frame);
        
        int keyboard_in = waitKey(50);
        if(keyboard_in == 27 || frame_ind == 5503 ) break;
    }
    video.release();
    return 0;
}
*/



