cmake_minimum_required(VERSION 3.29)
project(email_batch_deleting)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenSSL REQUIRED)

add_executable(email_batch_deleting main.cpp
        ConnectToImap.cpp
        ConnectToImap.h
        GetAndRefreshOAuth.cpp
        GetAndRefreshOAuth.h
        BasicServer.cpp
        BasicServer.h)

target_link_libraries(email_batch_deleting OpenSSL::SSL OpenSSL::Crypto)
