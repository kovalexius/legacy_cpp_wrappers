cmake_minimum_required(VERSION 2.8)

project(screenShot)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(DXSUPPORT "DirectX supprot screenshoting" OFF)

if(DXSUPPORT)
	find_package(DirectX9 REQUIRED)
    #include(DirectXConfig)
endif()


if(DXSUPPORT)
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
			jShot/src/ddraw_decode_error.h
		)
else()
	set(JSHOT_SOURCE
			jShot/src/GDIScreenShooter.cpp
			jShot/src/GDIScreenShooter.h
			jShot/src/types/geometry_types.cpp
			jShot/src/types/geometry_types.h
			jShot/src/ddraw_decode_error.cpp
			jShot/src/ddraw_decode_error.h
		)
endif()

if(DXSUPPORT)
	include_directories(./jpeg-9b/ 
					SYSTEM ${DIRECTX_INCLUDE_DIRS})
else()
	include_directories(./jpeg-9b/)
endif()
		
add_library(jShot STATIC 
				${JSHOT_SOURCE})

if(DXSUPPORT)
	target_link_libraries(jShot
							${DIRECTX_LIBRARIES_PATHS})
else()
endif()

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