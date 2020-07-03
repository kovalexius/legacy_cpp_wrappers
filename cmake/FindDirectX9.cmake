# Find DirectX9

message("Looking for DirectX9...")

file(TO_CMAKE_PATH $ENV{DXSDK_DIR} DxSdkDir)

file(GLOB DX9_SEARCH_INCLUDE_PATHS
	"${DxSdkDir}"
	"${DxSdkDir}/Include"
	"$ENV{PROGRAMFILES}/Microsoft DirectX SDK*/Include"
	"$ENV{PROGRAMFILES}/Microsoft SDKs/Windows/*/Include"
	"C:/Program Files (x86)/Windows Kits/*/include/um"
	"C:/Program Files/Windows Kits/*/include/um"
	"C:/Program Files (x86)/Windows Kits/10/Include/10.0.*.0/um"
	"C:/Program Files/Windows Kits/10/Include/10.0.*.0/um"
)
message(STATUS "DX9_SEARCH_INCLUDE_PATHS: ${DX9_SEARCH_INCLUDE_PATHS}")
find_path(DIRECTX_INCLUDE_DIRS
          NAMES "d3d9.h"
		  PATH_SUFFIXES "include"
          PATHS 
			${DX9_SEARCH_INCLUDE_PATHS} 
			${DxSdkDir}
			)



file(GLOB DX9_SEARCH_LIBRARY_PATHS
	"${DxSdkDir}"
	"${DxSdkDir}/Lib"
	"$ENV{PROGRAMFILES}/Microsoft DirectX SDK*/Lib"
	"$ENV{PROGRAMFILES}/Microsoft SDKs/Windows/*/Lib"
	"C:/Program Files (x86)/Windows Kits/*/Lib/10.0.*.0/um"
	"C:/Program Files/Windows Kits/*/Lib/10.0.*.0/um"
	"C:/Program Files (x86)/Windows Kits/10/Lib/10.0.*.0/um"
	"C:/Program Files/Windows Kits/10/Lib/10.0.*.0/um"
)
message(STATUS "DX9_SEARCH_LIBRARY_PATHS: ${DX9_SEARCH_LIBRARY_PATHS}")

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	find_library(D3D9_LIB 
					NAMES "d3d9"
					PATH_SUFFIXES "/x64/"
					PATHS	
						${DX9_SEARCH_LIBRARY_PATHS}
						${DxSdkDir}/Lib/
					NO_DEFAULT_PATH)
	find_library(D3DX9_RELEASE_LIB 
					NAMES "d3dx9"
					PATH_SUFFIXES "/x64/"
					PATHS 
						${DX9_SEARCH_LIBRARY_PATHS}
						${DxSdkDir}/Lib/
					NO_DEFAULT_PATH)
	find_library(D3DX9_DEBUG_LIB 
					NAMES "d3dx9d"
					PATH_SUFFIXES "/x64/"
					PATHS 
						${DX9_SEARCH_LIBRARY_PATHS}
						${DxSdkDir}/Lib/
					NO_DEFAULT_PATH)
	find_library(DDRAW_LIB 
					NAMES "ddraw"
					PATH_SUFFIXES "/x64/"
					PATHS	
						${DX9_SEARCH_LIBRARY_PATHS}
						${DxSdkDir}/Lib/
					NO_DEFAULT_PATH)
else()
	find_library(D3D9_LIB 
					NAMES "d3d9"
					PATH_SUFFIXES "/x86/"
					PATHS	
						${DX9_SEARCH_LIBRARY_PATHS}
						${DxSdkDir}/Lib/
					NO_DEFAULT_PATH)
	find_library(D3DX9_RELEASE_LIB 
					NAMES "d3dx9"
					PATH_SUFFIXES "/x86/"
					PATHS 
						${DX9_SEARCH_LIBRARY_PATHS}
						${DxSdkDir}/Lib/
					NO_DEFAULT_PATH)
	find_library(D3DX9_DEBUG_LIB 
					NAMES "d3dx9d"
					PATH_SUFFIXES "/x86/"
					PATHS 
						${DX9_SEARCH_LIBRARY_PATHS}
						${DxSdkDir}/Lib/
					NO_DEFAULT_PATH)
	find_library(DDRAW_LIB 
					NAMES "ddraw"
					PATH_SUFFIXES "/x86/"
					PATHS	
						${DX9_SEARCH_LIBRARY_PATHS}
						${DxSdkDir}/Lib/
					NO_DEFAULT_PATH)
endif()


 
set(DIRECTX_LIBRARIES 
		${D3D9_LIB} 
		optimized ${D3DX9_RELEASE_LIB}
		debug ${D3DX9_DEBUG_LIB}
		${DDRAW_LIB})

message("Include: ${DIRECTX_INCLUDE_DIRS}")
message("Libs: ${DIRECTX_LIBRARIES}")

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(DIRECTX9
    DEFAULT_MSG DIRECTX_ROOT_DIR
    DIRECTX_LIBRARIES DIRECTX_INCLUDE_DIRS
)
mark_as_advanced(DIRECTX_INCLUDE_DIRS DX9_LIB)