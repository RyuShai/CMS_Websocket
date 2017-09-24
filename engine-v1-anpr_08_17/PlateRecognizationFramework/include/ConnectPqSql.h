#include "iostream"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <fstream>
#include <ctime>
#include <thread>
#include <chrono>
#include <pqxx/pqxx>
#include <cstring>
#include <string>

using namespace std;
using namespace cv;
using namespace pqxx;

class ConnectPqDB
{
  public:
    ConnectPqDB();
    connection *C;
    ~ConnectPqDB();
};