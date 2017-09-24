project(PlateExtractor VERSION 1.0.0)

include_directories(./include ../PlateDetector/include)

file(GLOB PlateExtractor_SRC
    "src/*.cpp"
)

add_library(PlateExtractor
    ${PlateExtractor_SRC}
)
