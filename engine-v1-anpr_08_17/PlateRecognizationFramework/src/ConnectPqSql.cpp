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

#include "ConnectPqSql.h"

using namespace std;
using namespace cv;
using namespace pqxx;

ConnectPqDB::ConnectPqDB()
{
    C = new connection("dbname = vehicle_recognization user = postgres password = 12345678a@ \
      hostaddr = 127.0.0.1 port = 5432");

    if (C->is_open())
    {
        // cout << "Opened database successfully: " << C->dbname() << endl;
    }
    else
    {
        cout << "Can't open database" << endl;
    }
}

ConnectPqDB::~ConnectPqDB()
{
    C->disconnect();
}