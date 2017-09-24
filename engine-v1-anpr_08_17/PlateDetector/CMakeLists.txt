project(PlateDetector VERSION 1.0.0)

include_directories(./include)

add_library(PlateDetector
    src/CascadeTrainingStrategy.cpp
    src/IPlateDetect.cpp
    src/IPlateDetectStrategy.cpp
    src/PlateDetector.cpp
)
