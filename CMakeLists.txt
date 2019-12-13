CMAKE_MINIMUM_REQUIRED(VERSION 2.8) 	#最低版本要求

#指定编译选项
set(CMAKE_BUILD_TYPE Debug )

#指定版本信息
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CPP_COMPILER g++)
SET(CMAKE_CXX_COMPILER g++)		    #设置g++编译器
if(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE "Release")
endif()
ADD_DEFINITIONS(
-g
-Wall
 -fpermissive
)



SET(CXX_FLAGS
    # -Wall
    # -Wextra
    # -Werror
    # -fpermissive
    # -Wno-unused-parameter
    # -std=c++11
    # -lpthread 
    # -lboost_regex
    # -levent 
    # -lm 
    # -lssl 
    # -lcrypto 
    # -levent_openssl 
    # -lstdc++ 
    # -lboost_system
 )

#SET(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
#SET(CMAKE_CXX_FLAGS_RELEASE "-O2 -finline-limit=1000 -DNDEBUG")

#指定编译目录
set(PROJECT_BINARY_DIR ${PROJECT_SOURCE_DIR}/build)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})
	
PROJECT(https_server)				            #设置工程名
MESSAGE(STATUS "Project HTTPS Server")	 	    #打印消息

#添加子目录,这样进入源码文件src目录可以继续构建
add_subdirectory(${PROJECT_SOURCE_DIR}/src)

# ADD_EXECUTABLE(hello hello.cpp)         #生成可执行文件

