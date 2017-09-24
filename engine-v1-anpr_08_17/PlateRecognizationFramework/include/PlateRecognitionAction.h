#include <queue>
#include <stack>
#include <mutex>
#include <chrono>
#include <string>

#include "PlateRecognizator.h"
#include "TextReader.h"
#include "ObjectUtils.h"
namespace pr
{
class PlateRecognitionAction
{
public:
  PlateRecognitionAction(
              cv::Rect cropRect,
              QueuePlates &plateQueue_);
  void operator()();

public:
  //pr::CarTextIsolation *isolator;
  Rect cropRect;
  QueuePlates &plateQueue;
  pr::TextReader *dp;
  pr::ObjectUtils objects;
};
}