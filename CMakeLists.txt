cmake_minimum_required(VERSION 3.11)
project(HAPusher-Examples)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(pusher) # main program
add_subdirectory(apache-zookeeper/zookeeper-client/zookeeper-client-c) # zookeeper