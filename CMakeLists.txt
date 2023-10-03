cmake_minimum_required(VERSION 3.19.5)
project(modem_host)

set(CMAKE_CXX_STANDARD 20)

set(SOURCES
    Logic.cpp
    CellularManager.cpp
    main.cpp 
)

add_executable(modem_managed ${SOURCES})
