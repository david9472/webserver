cmake_minimum_required(VERSION 3.28)
project(webserver)

include(FetchContent)

FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt
        GIT_TAG        e69e5f977d458f2650bb346dadf2ad30c5320281) # 10.2.1
FetchContent_MakeAvailable(fmt)

set(CMAKE_CXX_STANDARD 17)

add_executable(webserver main.cpp
        serializable.hpp
        ipaddress.hpp)

target_link_libraries(webserver fmt::fmt)
