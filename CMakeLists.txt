cmake_minimum_required(VERSION 2.8)

project(screenShot)

set(JSHOT
		jShot/src/CJpegCompress.cpp
		jShot/src/CJpegCompress.h
		jShot/src/CJScreenShot.cpp
		jShot/src/CJScreenShot.h
		jShot/src/jDll1.cpp
		jShot/src/jDll.h
		jShot/src/myMgr.h
		jShot/src/LibJpeg.cpp
		jShot/src/LibJpeg.h
		jShot/src/LibJpeg1.h)
		
add_library(jShot SHARED ${JSHOT})

set (JTHREADPOOL 
		JThreadPool/src/CJThreadPool.cpp
		JThreadPool/src/CJThreadPool.h)
		
add_library(jThreadPool STATIC ${JTHREADPOOL})

add_executable(testJShot
				testJShot/testJShot.cpp)
				
target_link_libraries(testJShot
							${jShot}
							${jThreadPool})