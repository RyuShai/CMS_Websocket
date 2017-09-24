cmake_minimum_required(VERSION 3.2)
project(PlateReader)

include(CTest)
enable_testing()


find_package( OpenCV REQUIRED )
add_definitions(-std=c++11 -Wall)

subdirs(
    PlateDetector
    PlateExtractor
    PlateRecognizationFramework
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

