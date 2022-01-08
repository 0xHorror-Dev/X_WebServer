cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project("WebServerExample")

add_library(RequestHandlePlugin SHARED "src/main.cpp" "src/Http/Http.h" "src/Http/Http.cpp")

target_link_libraries(RequestHandlePlugin PRIVATE XPlatform.core XPlatform.net-headers)