cmake_minimum_required(VERSION 3.12)
project(cvToyMachine)
find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED)

set(CMAKE_CXX_STANDARD 14)

include_directories(${Boost_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})
add_executable(cvToyMachine projection.cpp)
target_link_libraries(cvToyMachine ${OpenCV_LIBS} ${Boost_LIBRARIES})