cmake_minimum_required(VERSION 3.6)
project(Test)

find_package( OpenCV REQUIRED )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)
add_executable(Test ${SOURCE_FILES})
target_link_libraries( Test ${OpenCV_LIBS} )