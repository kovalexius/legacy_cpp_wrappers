cmake_minimum_required(VERSION 2.8)

project(jShot)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")

option(DXSUPPORT "DirectX supprot screenshoting" ON)

if(DXSUPPORT)
	find_package(DirectX9 REQUIRED)
endif()


if(DXSUPPORT)
	set(JSHOT_SOURCE
			./src/GDIScreenShooter.cpp
			./src/GDIScreenShooter.h
			./src/DXScreenShooter.cpp
			./src/DXScreenShooter.h
			./src/DDrawScreenShooter.cpp
			./src/DDrawScreenShooter.h
			./src/types/geometry_types.cpp
			./src/types/geometry_types.h
			./src/ddraw_decode_error.cpp
			./src/ddraw_decode_error.h
		)
else()
	set(JSHOT_SOURCE
			./src/GDIScreenShooter.cpp
			./src/GDIScreenShooter.h
			./src/types/geometry_types.cpp
			./src/types/geometry_types.h
		)
endif()

if(DXSUPPORT)
	include_directories(
					 ${DIRECTX_INCLUDE_DIR})
endif()
		
add_library(jShot STATIC 
				${JSHOT_SOURCE})

if(DXSUPPORT)
	target_link_libraries(jShot
							${DIRECTX_LIBRARIES_PATHS})
endif()
