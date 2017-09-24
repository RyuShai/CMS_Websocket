#include <queue>
#include <stack>
#include <mutex>
#include <chrono>
#include <string>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "PlateRecognizator.h"
#include "ObjectUtils.h"
#include "crop_plate_corner.h"


namespace pr
{

class PlateReader
{
public:
  PlateReader(cv::Rect cropRect,
              std::queue<FrameData> &frameQueue_, std::mutex &frameMutex_,
              QueuePlates &plateQueue_,
              std::queue<PlateData> &plateQueue_showFrame_,std::mutex &plateQueue_showFrameMutex_);
  void operator()();

public:
  
  std::queue<FrameData> &frameQueue;
  std::mutex &frameMutex;
  QueuePlates &plateQueue;
  std::queue<PlateData> &plateQueue_showFrame;
  std::mutex &plateQueue_showFrameMutex;

  cv::Rect cropRect;
  pr::PlateRecognizator recognizator_vehicle[MaxKindsPlate];
  int QueueSize;
  int CropPlate;
  ObjectUtils objects;
};
}