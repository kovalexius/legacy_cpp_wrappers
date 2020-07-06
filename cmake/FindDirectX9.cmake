
file(TO_CMAKE_PATH $ENV{DXSDK_DIR} DXSDKDIR)
file(TO_CMAKE_PATH $ENV{PROGRAMFILES} PROGRAMFILESDIR)

file(GLOB DX9_SEARCH_INCLUDE_PATHS
	"${DXSDKDIR}"
	"${DXSDKDIR}/Include"
	"${PROGRAMFILESDIR}/Microsoft DirectX SDK*/Include"
	"${PROGRAMFILESDIR}/Microsoft SDKs/Windows/*/Include"
	"C:/Program Files (x86)/Windows Kits/*/include/um"
	"C:/Program Files/Windows Kits/*/include/um"
	"C:/Program Files (x86)/Windows Kits/10/Include/10.0.*.0/um"
	"C:/Program Files/Windows Kits/10/Include/10.0.*.0/um"
)
#message(STATUS "DX9_SEARCH_INCLUDE_PATHS: ${DX9_SEARCH_INCLUDE_PATHS}")

foreach(current_dir ${DX9_SEARCH_INCLUDE_PATHS})
    find_path(DIRECTX_INCLUDE_DIR
            NAMES
                d3d9.h
            PATHS
                ${current_dir}
            PATH_SUFFIXES 
                "include"
            NO_DEFAULT_PATH
            NO_CMAKE_PATH
            )
    if(DIRECTX_INCLUDE_DIR STREQUAL "DIRECTX_INCLUDE_DIR-NOTFOUND")
        message(STATUS "current_dir: ${current_dir}")
        message(STATUS "DIRECTX_INCLUDE_DIRS: ${DIRECTX_INCLUDE_DIR}")
        unset(DIRECTX_INCLUDE_DIRS CACHE)
    else()
        break()
    endif()
endforeach()


set(DirectX_LIBRARIES "d3d9" "d3dx9" "ddraw")
file(GLOB DX9_SEARCH_LIBRARY_PATHS
	"${DXSDKDIR}"
	"${DXSDKDIR}/Lib"
	"${ProgramFilesDir}/Microsoft DirectX SDK*/Lib"
	"${ProgramFilesDir}/Microsoft SDKs/Windows/*/Lib"
    "C:/Program Files (x86)/Microsoft DirectX SDK (August 2007)/Lib/"
    "C:/Program Files (x86)/Microsoft DirectX SDK*/Lib/"
	"C:/Program Files (x86)/Windows Kits/*/Lib/10.0.*.0/um"
	"C:/Program Files/Windows Kits/*/Lib/10.0.*.0/um"
	"C:/Program Files (x86)/Windows Kits/10/Lib/10.0.*.0/um"
	"C:/Program Files/Windows Kits/10/Lib/10.0.*.0/um"
)
message(STATUS "DX9_SEARCH_LIBRARY_PATHS: ${DX9_SEARCH_LIBRARY_PATHS}")


set(Bitdepth_SUFFIX "/x86")
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(Bitdepth_SUFFIX "/x64")
endif()

set(Lib_FOUND 1)
foreach(current_lib ${DirectX_LIBRARIES})
	message(STATUS "current_lib: ${current_lib}")
	foreach(current_dir ${DX9_SEARCH_LIBRARY_PATHS})
		message(STATUS "current_dir: ${current_dir}")
		unset(PATH_LIB CACHE)
        find_library(PATH_LIB
					NAMES ${current_lib}
					PATH_SUFFIXES ${Bitdepth_SUFFIX}
					PATHS ${current_dir}
					NO_DEFAULT_PATH
                    NO_CMAKE_PATH
                    NO_CMAKE_SYSTEM_PATH)
        
        message(STATUS "PATH_LIB: ${PATH_LIB}")
        
        if(${PATH_LIB} STREQUAL "PATH_LIB-NOTFOUND")
            #unset(DIRECTX_LIBRARIES_PATHS)
        else()
            set(DIRECTX_LIBRARIES_PATHS ${DIRECTX_LIBRARIES_PATHS} ${PATH_LIB})
			break()
        endif()
        
        
    endforeach()
	
    message(STATUS "")
    #if(DIRECTX_LIBRARIES_PATHS)
    #    break()
    #endif()
endforeach()

message(STATUS "DIRECTX_INCLUDE_DIR: ${DIRECTX_INCLUDE_DIR}")
message(STATUS "DIRECTX_LIBRARIES_PATHS: ${DIRECTX_LIBRARIES_PATHS}")

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(DIRECTX9 
                                    DEFAULT_MSG
                                        DIRECTX_INCLUDE_DIR
                                        DIRECTX_LIBRARIES_PATHS)