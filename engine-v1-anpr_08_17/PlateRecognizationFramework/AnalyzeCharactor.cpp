#include "iostream"
#include <ctime>
#include <fstream>
#include <pqxx/pqxx>
#include <cstring>
#include <string>

#include "ConnectPqSql.cpp"
#include "ListProvince.cpp"
#include "ListDistrict.cpp"
#include "ListMilitary.cpp"
#include "ListNation.cpp"

std::string camera_id="1";
std::string location="GIANG VAN MINH, HA NOI";

void writeDataToDB(std::string plate, char *province,std::string image, std::string vehicleImage, char *type)
{
    char *provice = province;
    char *carType = type;
    string sql1;
    string sql2;
    ConnectPqDB *connect = new ConnectPqDB();
    // connect ->test(provice); // TODO: sonhh will be fix after version 0.1
    connection *C = connect->C;
    try
    {
        if (C->is_open())
        {
            char buff[20];
            time_t now = time(NULL);
            strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&now));

            sql1 = "INSERT INTO tbl_vehicle_base (vehicle_plate, vehicle_type, plate_type, province) "
                  "VALUES ('";
            sql1.append(plate);
            sql1.append("','");
            sql1.append(carType);
            sql1.append("', 'white','");
            sql1.append(provice);
            sql1.append("')");

            sql2 = "INSERT INTO tbl_vehicle_location (vehicle_plate, camera_id, time, plate_image, vehicle_image, location) VALUES ('";
            sql2.append(plate);
            sql2.append("','");
            sql2.append(camera_id);
            sql2.append("','");
            sql2.append(buff);
            sql2.append("','");
            sql2.append(image);
            sql2.append("','");
            sql2.append(vehicleImage);
            sql2.append("','");
            sql2.append(location);
            sql2.append("')");

            delete[] provice;
            try {
                work W(*C);
                W.exec(sql1);
                W.exec(sql2);
                W.commit();
                cout<<"Successed to insert new vehicle & new location"<<endl;
            } catch (const pqxx::sql_error &e) {
                // std::cerr << "Database error: " << e.what() << std::endl << "Query was: " << e.query() << std::endl;
                work W(*C);
                W.exec(sql2);
                W.commit();
                cout<<"Successed to insert new location for vehicle"<<endl;
            }
        }
        else
        {
            cout << "Can't open database" << endl;
        }
        C->disconnect();
    }
    catch (const std::exception &e)
    {
        // cerr << e.what() << std::endl;
    }
}

// void filterDataFromPlateCar(std::string fullText, std::string text,std::string image)
// {
//     std::string sql;
//     string split2charator = text.substr(0, 2);
//     std::string provinces = listProvince[split2charator];
//     char *provice = new char[provinces.length() + 1];
//     std::strcpy(provice, provinces.c_str());
//     writeDataToDB(fullText, provice, "car");
// }

// void filterDataFromPlateMotoBike(std::string fullText, std::string text,std::string image)
// {
//     std::string sql;
//     string split2charator = text.substr(0, 2);
//     std::string provinces = listProvince[split2charator];
//     char *provice = new char[provinces.length() + 1];
//     std::strcpy(provice, provinces.c_str());
//     writeDataToDB(fullText, provice, "moto");
// }

void filterProvince(std::string fullText,std::string text, std::string image, std::string vehicleImage,char *type)
{
    //std::string sql;
    string split2charator = text.substr(0, 2);
    std::string provinces = listProvince[split2charator];
    char *province = new char[provinces.length() + 1];
    std::strcpy(province, provinces.c_str());
    writeDataToDB(fullText, province, image, vehicleImage, type);
}

void analyzePlateText(std::string fullText,std::string image, std::string vehicleImage)
{
    std::string delimiter = "-";
    std::string prefix = fullText.substr(0, fullText.find(delimiter));
    if (prefix.length() == 3)
    {   
        filterProvince(fullText, prefix, image, vehicleImage, "car");
    }
    else
    {   
        // std::string prefix_type = prefix.substr(2, 2);
        // if(prefix_type=="LD"){
        //     filterProvince(fullText, prefix, image, vehicleImage, "car");
        // }else{
            filterProvince(fullText, prefix, image, vehicleImage, "moto");
        // }
    }
}
