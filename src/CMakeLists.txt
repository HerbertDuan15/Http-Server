#查找当前目录下的所有源文件，
#并将名称保存到DIR_LIB_SRCS目录
#aux_source_directory(. DIR_LIB_SRCS)
#SET(CMAKE_CXX_COMPILER g++)

#指定头文件目录,PROJECT_SOURCE_DIR为工程的根目录  
include_directories(${PROJECT_SOURCE_DIR}/include)

#指定可执行文件的输出目录，输出到bin下面  
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

ADD_DEFINITIONS(
-g
-Wall
 -fpermissive
)

SET(CXX_FLAGS
 -Wall
 -Wunused-variable
 -Wextra
 -Werror
 -Wno-unused-parameter
 -std=c++11
 -lpthread
 # -levent
 # -lm
 # -lssl
 # -lcrypto
 # -levent_openssl
 # -lstdc++
 # -lboost_system
 # -lboost_regex
 )

SET(SRCS
    https_action.cpp
    https_connection.cpp
    my_config.cpp
  )
#TARGET_LINK_LIBRARiES(boost pthread rt)
##生成动态库    
#add_library(shared_demo SHARED lib_demo.cpp)
##设置库输出名为 shared => libshared.so  
#set_target_properties(shared_demo PROPERTIES OUTPUT_NAME "shared") 
 
#生成静态库 
add_library(https_server SHARED ${SRCS}) 
#设置输库出名为 static => libstatic.a  
set_target_properties(https_server PROPERTIES OUTPUT_NAME "https_server") 

#指定库文件输出路径  
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

##在指定目录下查找库，并保存在LIBPATH变量中
#find_library(LIBPATHS  ${PROJECT_BINARY_DIR}/lib)  

add_executable(server main.cpp)
target_link_libraries(server https_server boost_system ssl pthread event m crypto event_openssl stdc++ jsoncpp boost_regex)
