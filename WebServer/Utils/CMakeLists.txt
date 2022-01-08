cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project("Utils")

add_library(XPlatform.Utils INTERFACE)
target_include_directories(XPlatform.Utils INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")