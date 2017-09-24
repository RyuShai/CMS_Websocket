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

#include "VLCReader.h"
#include "PlateRecognizator.h"
#include "vlc/vlc.h"
#include "CarTextIsolation.h"
#include "IOData.h"
#include "PlateRegion.h"

using namespace pr;
using namespace std;
using namespace cv;
using namespace pqxx;

class ConnectPqDB
{
  public:
    ConnectPqDB();
    void test(char *provice);
    connection *C;
    ~ConnectPqDB();
};

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

// void ConnectPqDB::test(char *provice)
// {
//     std::string sql;
//     try
//     {
//         if (C->is_open())
//         {
//             sql = "INSERT INTO tbl_vehicle_base (plate, vehicle_type, vehicle_size, plate_type,province) "
//                   "VALUES ('29k1-01204', 'car', '1212', 'white','";
//             sql.append(provice);
//             sql.append("')");
//             delete[] provice;

//             /* Create a transactional object. */
//             pqxx::work W(*C);

//             /* Execute SQL query */
//             W.exec(sql);
//             W.commit();
//             cout << "Records created successfully" << endl;
//         }
//         C->disconnect();
//     }
//     catch (const std::exception &e)
//     {
//         cerr << e.what() << std::endl;
//     }
// }
