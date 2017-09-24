#include <queue>
#include <stack>
#include <mutex>
#include <chrono>
#include <string>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "ObjectUtils.h"

namespace pr
{
class VideoReader
{
  public:
    VideoReader(std::string url, int width, int height, cv::Rect cropRect, std::queue<FrameData> &queue_);
    void operator()();

  public:
    std::string url;
    int width;
    int height;
    std::queue<FrameData> &frameQueue;
    pr::ObjectUtils objects;
};
}
