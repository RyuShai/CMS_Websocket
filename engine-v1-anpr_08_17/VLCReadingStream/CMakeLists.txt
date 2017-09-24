project(VLCReadingStream VERSION 1.0.0)

file(GLOB VLCReadingStream_SRC
    "*.cpp"
)

add_library(VLCReadingStream
    ${VLCReadingStream_SRC}
)
