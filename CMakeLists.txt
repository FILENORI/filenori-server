cmake_minimum_required(VERSION 3.10)
project(boost_asio_example)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Boost REQUIRED COMPONENTS system)
find_package(nlohmann_json REQUIRED) # nlohmann_json 추가

add_executable(boost_asio_example ./src/main.cpp)
target_link_libraries(boost_asio_example PRIVATE Boost::system nlohmann_json::nlohmann_json)
