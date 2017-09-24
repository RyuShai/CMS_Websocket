#include "iostream"
#include <fstream>
#include <pqxx/pqxx>
#include <cstring>
#include <string>

//TODO: will be read from file after release version 0.1
std::map<string, string> listProvince = {
        {"11", "Cao Bang"},
        {"12", "Lang Son"},
        {"13", "Ha Bac (cu)"},
        {"14", "Quang Ninh"},
        {"15", "TP Hai Phong"},
        {"16", "TP Hai Phong"},
        {"17", "Thai Binh"},
        {"18", "Nam Dinh"},
        {"19", "Phu Tho"},
        {"20", "Thai Nguyen"},
        {"21", "Yen Bai"},
        {"22", "Tuyen Quang"},
        {"23", "Ha Giang"},
        {"24", "Lao Cai"},
        {"25", "Lai Chau"},
        {"26", "Son La"},
        {"27", "Dien Bien"},
        {"28", "Hoa Binh"},
        {"29", "TP Ha Noi"},
        {"30", "TP Ha Noi"},
        {"31", "TP Ha Noi"},
        {"32", "TP Ha Noi"},
        {"33", "TP Ha Noi"},
        {"34", "Hai Duong"},
        {"35", "Ninh Binh"},
        {"36", "Thanh Hoa"},
        {"37", "Nghe An"},
        {"38", "Ha Tinh"},
        {"39", "Dong Nai"},
        {"40", "TP Ha Noi"},
        {"41", "TP Ho Chi Minh"},
        {"43", "Da Nang"},
        {"47", "Dak Lak"},
        {"48", "Dak Nong"},
        {"49", "Lam Dong"},
        {"50", "TP Ho Chi Minh"},
        {"51", "TP Ho Chi Minh"},
        {"52", "TP Ho Chi Minh"},
        {"53", "TP Ho Chi Minh"},
        {"54", "TP Ho Chi Minh"},
        {"55", "TP Ho Chi Minh"},
        {"56", "TP Ho Chi Minh"},
        {"57", "TP Ho Chi Minh"},
        {"58", "TP Ho Chi Minh"},
        {"59", "TP Ho Chi Minh"},
        {"60", "Dong Nai"},
        {"61", "Binh Duong"},
        {"62", "Long An"},
        {"63", "Tien Giang"},
        {"64", "Vinh Long"},
        {"65", "TP Can Tho"},
        {"66", "Dong Thap"},
        {"67", "An Giang"},
        {"68", "Kien Giang"},
        {"69", "Ca Mau"},
        {"70", "Tay Ninh"},
        {"71", "Ben Tre"},
        {"72", "Ba Ria - Vung Tau"},
        {"73", "Quang Binh"},
        {"74", "Quang Tri"},
        {"75", "Thua Thien - Hue"},
        {"76", "Quang Ngai"},
        {"77", "Binh Dinh"},
        {"78", "Phu Yen"},
        {"79", "Khanh Hoa"},
        {"81", "Gia Lai"},
        {"82", "Kon Tum"},
        {"83", "Soc Trang"},
        {"84", "Tra Vinh"},
        {"85", "Ninh Thuan"},
        {"86", "Binh Thuan"},
        {"88", "Vinh Phuc"},
        {"89", "Hung Yen"},
        {"90", "Ha Nam"},
        {"92", "Quang Nam"},
        {"93", "Binh Phuoc"},
        {"94", "Bac Lieu"},
        {"95", "Hau Giang"},
        {"97", "Bac Kan"},
        {"98", "Bac Giang"},
        {"99", "Bac Ninh"}};