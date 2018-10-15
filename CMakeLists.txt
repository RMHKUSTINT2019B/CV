cmake_minimum_required(VERSION 3.9)
project(CV)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenCV REQUIRED)

file(GLOB_RECURSE SRC Source/*)

add_executable(CV ${SRC})

target_include_directories(CV PUBLIC ${OpenCV_INCLUDE_DIRS})
target_link_libraries(CV ${OpenCV_LIBS})
