project(PlateRecognizationFramework VERSION 1.0.0)

include_directories(
    ./include 
    ../PlateDetector/include 
    ../PlateExtractor/include 
)

add_executable(PlateRecognizationFramework
    Main.cpp
    src/IOData.cpp
    src/ObjectUtils.cpp
    src/VideoReader.cpp
    src/PlateRecognizator.cpp
    src/PlateReader.cpp
    src/PlateRecognitionAction.cpp
    src/ConnectPqSql.cpp
    src/AnalyzeCharactor.cpp
    src/crop_plate_corner.cpp
    src/crop_character_rect.cpp
    src/group_plates.cpp
    src/trackingkalman.cpp
    src/HungarianAlg.cpp
    src/Kalman.cpp
    src/Qtrackers.cpp
    src/estimate_square_cascade.cpp
)
SET(GCC_COVERAGE_COMPILE_FLAGS "-I /usr/local/include -L /usr/local/lib -lpqxx -lpq")

target_link_libraries(PlateRecognizationFramework
    PlateDetector
    PlateExtractor
    ${OpenCV_LIBS}
    ${GCC_COVERAGE_COMPILE_FLAGS}
    pthread
    vlc
    jsoncpp
    json
)
