#1.cmake verson，指定cmake版本 
cmake_minimum_required(VERSION 3.9)

#2.project name，指定项目的名称，一般和项目的文件夹名称对应
project(opencv-demo)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)

macro(source_group_by_dir source_files)
if(MSVC)
set(sgbd_cur_dir ${CMAKE_CURRENT_SOURCE_DIR})
foreach(sgbd_file ${${source_files}})
string(REGEX REPLACE ${sgbd_cur_dir}/\(.*\) \\1 sgbd_fpath ${sgbd_file})
string(REGEX REPLACE "\(.*\)/.*" \\1 sgbd_group_name ${sgbd_fpath})
string(COMPARE EQUAL ${sgbd_fpath} ${sgbd_group_name} sgbd_nogroup)
string(REPLACE "/" "\\" sgbd_group_name ${sgbd_group_name})
if(sgbd_nogroup)
set(sgbd_group_name "\\")
endif(sgbd_nogroup)
source_group(${sgbd_group_name} FILES ${sgbd_file})
endforeach(sgbd_file)
endif(MSVC)
endmacro(source_group_by_dir)

if(WIN32)
	add_definitions(-DOS_WINDOWS -D_CRT_SECURE_NO_WARNINGS)
	SET(PREFIX "lib")
	SET(THREAD "")
elseif(UNIX)
    add_definitions(-DOS_LINUX -DDEBUG)
	SET(PREFIX "")
	SET(THREAD "pthread")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

include_directories(
./third_party/opencv/include
)

file(GLOB_RECURSE DIR_TEST_SRCS "test.cpp")
file(GLOB_RECURSE DIR_FISHEYE_SRCS "fisheye_undistort.cpp")
file(GLOB_RECURSE DIR_FISHEYE_CALIB_SRCS "fisheye_calibrate.cpp")
file(GLOB_RECURSE OPENCV_LIB "${PROJECT_SOURCE_DIR}/third_party/opencv/lib/opencv_world480.lib") 

set(all_files ${DIR_TEST_SRCS} ${DIR_FISHEYE_SRCS} ${DIR_FISHEYE_CALIB_SRCS})
source_group_by_dir(all_files)

add_executable(demo ${DIR_TEST_SRCS})
target_link_libraries(demo ${OPENCV_LIB})

add_executable(fisheye_undistort ${DIR_FISHEYE_SRCS})
target_link_libraries(fisheye_undistort ${OPENCV_LIB})

add_executable(fisheye_calibrate ${DIR_FISHEYE_CALIB_SRCS})
target_link_libraries(fisheye_calibrate ${OPENCV_LIB})

set(OPENCV_DLL "${PROJECT_SOURCE_DIR}/third_party/opencv/bin/opencv_world480.dll")
if (WIN32 AND EXISTS "${OPENCV_DLL}")
	add_custom_command(TARGET demo POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${OPENCV_DLL}"
			"$<TARGET_FILE_DIR:demo>/opencv_world480.dll"
	)
	add_custom_command(TARGET fisheye_undistort POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${OPENCV_DLL}"
			"$<TARGET_FILE_DIR:fisheye_undistort>/opencv_world480.dll"
	)
	add_custom_command(TARGET fisheye_calibrate POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${OPENCV_DLL}"
			"$<TARGET_FILE_DIR:fisheye_calibrate>/opencv_world480.dll"
	)
endif()

set(CMAKE_BUILD_TYPE=Debug)
