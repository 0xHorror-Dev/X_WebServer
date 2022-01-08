cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project("WebServerExample")

add_subdirectory(RequestHandlePlugin)

add_executable(WebServerExample "src/main.cpp"
"src/WebServerApplication/WebServerApplication.h" "src/WebServerApplication/WebServerApplication.cpp" )

XPLATFORM_ADD_DEPENDENCE(WebServerExample XPlatform.core)
XPLATFORM_ADD_DEPENDENCE_MODULE(WebServerExample XPlatform.net)

XPLATFORM_COPY_FILE(WebServerExample "${CMAKE_CURRENT_SOURCE_DIR}/configs/project.json" $<TARGET_FILE_DIR:WebServerExample>)
XPLATFORM_COPY_FILE(WebServerExample "${CMAKE_CURRENT_SOURCE_DIR}/configs/XPlatform.json" $<TARGET_FILE_DIR:WebServerExample>/Engine/XPlatform.json)
XPLATFORM_COPY_FOLDER(WebServerExample "${CMAKE_CURRENT_SOURCE_DIR}/certs/" $<TARGET_FILE_DIR:WebServerExample>/certs/)

add_subdirectory("Utils")

target_link_libraries(WebServerExample XPlatform.core XPlatform.net-headers XPlatform.Utils)

if(UNIX)
  find_package(Threads REQUIRED)
  target_link_libraries(WebServerExample ${CMAKE_THREAD_LIBS_INIT})
  message("-- [xplatform]: thread: ${CMAKE_THREAD_LIBS_INIT}")
  target_compile_options(WebServerExample PRIVATE -fpermissive)
endif(UNIX)

if(WIN32)
target_link_libraries(WebServerExample wsock32 ws2_32)
endif(WIN32)
