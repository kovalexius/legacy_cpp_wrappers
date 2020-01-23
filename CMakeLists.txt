cmake_minimum_required(VERSION 2.8)

project(screenShot)

set(DX_SDK
		$ENV{DXSDK_DIR})

if(IS_DIRECTORY ${DX_SDK})
else()
	message(WARNING "$(DXSDK_DIR) is not directory or not defined; DXSDK_DIR = ${DX_SDK}")
endif()

set(JSHOT_SOURCE
		jShot/src/GDIScreenShooter.cpp
		jShot/src/GDIScreenShooter.h
		jShot/src/DXScreenShooter.cpp
		jShot/src/DXScreenShooter.h
        jShot/src/DDrawScreenShooter.cpp
		jShot/src/DDrawScreenShooter.h
		jShot/src/types/geometry_types.cpp
		jShot/src/types/geometry_types.h
		jShot/src/ddraw_decode_error.cpp
		jShot/src/ddraw_decode_error.h)
		
include_directories(./jpeg-9b/ 
					SYSTEM ${DX_SDK}/Include)
		
add_library(jShot STATIC ${JSHOT_SOURCE})
target_link_libraries(jShot
							D3d9.lib
							${DX_SDK}/Lib/x64/d3dx9.lib
							Ddraw.lib)

set (JTHREADPOOL_SOURCE 
		JThreadPool/src/CJThreadPool.cpp
		JThreadPool/src/CJThreadPool.h)
		
add_library(jThreadPool STATIC ${JTHREADPOOL_SOURCE})

add_executable(testJShot
				tests/testGDI.cpp)
				
target_link_libraries(testJShot
							jShot
							jThreadPool
							)