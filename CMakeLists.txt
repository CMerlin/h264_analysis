PROJECT(rmu)

cmake_minimum_required(VERSION 2.6)

#set(CMAKE_C_COMPILER /home/julyrain/toolschain/arm-2008q1/bin/arm-linux-gcc)
#set(CMAKE_C_COMPILER arm-linux-gcc)
##设定交叉编译工具
set(CMAKE_C_COMPILER arm-linux-g++)
#可执行程序存放路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
#编译的选项
set(CMAKE_C_FLAGS "-Wall -g")
add_compile_options(-std=c++11)
#add_definitions(-DUSING_TIMERFD)
#add_definitions(-DUSING_ZIP)

#头文件
include_directories(${PROJECT_SOURCE_DIR}/include)
#${PROJECT_SOURCE_DIR}/components/include
	#				${PROJECT_SOURCE_DIR}/utilities/tools)
#.CC文件
add_subdirectory(src)
#add_subdirectory(utilities)
