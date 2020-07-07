cmake_minimum_required(VERSION 2.8)

project(screenShot)

option(TESTONLY "Build only test without jpeg compression, without dll module" ON)

add_subdirectory(jShot/ jShot
					EXCLUDE_FROM_ALL)

if(NOT TESTONLY)
	add_subdirectory(JThreadPool/ jThreadPool
						EXCLUDE_FROM_ALL)
endif()
		
include_directories(
					 ${DIRECTX_INCLUDE_DIR})

if(TESTONLY)
	add_executable(testJShot
				tests/testGDI.cpp)
else()
	include_directories(./jpeg-9b/ )
	add_executable(testJShot
				tests/testJShot.cpp)
endif()

if(TESTONLY)	
	target_link_libraries(testJShot
							jShot
							)
elseif()
	target_link_libraries(testJShot
							jShot
							jThreadPool
							)
endif()

