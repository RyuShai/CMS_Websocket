#include "iostream"
#include <fstream>
#include <pqxx/pqxx>
#include <cstring>
#include <string>

//TODO: will be read from file after release version 0.1
std::map<string, string> listMilitary = {
        {"AA", "Quan doan 1 - Binh doan Quyet Thang"},
        {"AB", "Quan doan 2 - Binh doan Huong Giang"},
        {"AC", "Quan doan 3 - Binh doan Tay Nguyen"}};
