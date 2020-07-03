#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# -----------------------------------------------------------------------------
# Find DirectX9 SDK
# Define:
# DirectX9_FOUND
# DirectX9_INCLUDE_DIRS
# DirectX9_LIBRARIES

if(WIN32) # The only platform it makes sense to check for DirectX9 SDK
	include(FindPkgMacros)
	findpkg_begin(DirectX9)

	# Get path, convert backslashes as ${ENV_DXSDK_DIR}
	getenv_path(DXSDK_DIR)
	getenv_path(DIRECTX_HOME)
	getenv_path(DIRECTX_ROOT)
	getenv_path(DIRECTX_BASE)

	# construct search paths
	set(DirectX9_PREFIX_PATH 
	"${DXSDK_DIR}" "${ENV_DXSDK_DIR}"
	"${DIRECTX_HOME}" "${ENV_DIRECTX_HOME}"
	"${DIRECTX_ROOT}" "${ENV_DIRECTX_ROOT}"
	"${DIRECTX_BASE}" "${ENV_DIRECTX_BASE}"
	"C:/apps_x86/Microsoft DirectX SDK*"
	"C:/Program Files (x86)/Microsoft DirectX SDK*"
	"C:/apps/Microsoft DirectX SDK*"
	"C:/Program Files/Microsoft DirectX SDK*"
	"$ENV{ProgramFiles}/Microsoft DirectX SDK*"
	)

	create_search_paths(DirectX9)
	# redo search if prefix path changed
	clear_if_changed(DirectX9_PREFIX_PATH
		DirectX9_LIBRARY
		DirectX9_INCLUDE_DIR
	)
  
	# dlls are in DirectX9_ROOT_DIR/Developer Runtime/x64|x86
	# lib files are in DirectX9_ROOT_DIR/Lib/x64|x86
	if(CMAKE_CL_64)
		set(DirectX9_LIBPATH_SUFFIX "x64")
	else(CMAKE_CL_64)
		set(DirectX9_LIBPATH_SUFFIX "x86")
	endif(CMAKE_CL_64)

	# look for D3D9 components
  find_path(DirectX9_INCLUDE_DIR NAMES D3D9.h HINTS ${DirectX9_INC_SEARCH_PATH})
  find_library(DirectX9_DXERR_LIBRARY NAMES DxErr HINTS ${DirectX9_LIB_SEARCH_PATH} PATH_SUFFIXES ${DirectX9_LIBPATH_SUFFIX})
	find_library(DirectX9_DXGUID_LIBRARY NAMES dxguid HINTS ${DirectX9_LIB_SEARCH_PATH} PATH_SUFFIXES ${DirectX9_LIBPATH_SUFFIX})
	find_library(DirectX9_DXGI_LIBRARY NAMES dxgi HINTS ${DirectX9_LIB_SEARCH_PATH} PATH_SUFFIXES ${DirectX9_LIBPATH_SUFFIX})
	find_library(DirectX9_D3DCOMPILER_LIBRARY NAMES d3dcompiler HINTS ${DirectX9_LIB_SEARCH_PATH} PATH_SUFFIXES ${DirectX9_LIBPATH_SUFFIX})
	find_library(DirectX9_D3D9_LIBRARY NAMES d3d9 HINTS ${DirectX9_LIB_SEARCH_PATH} PATH_SUFFIXES ${DirectX9_LIBPATH_SUFFIX})
	find_library(DirectX9_D3DX9_LIBRARY NAMES d3dx9 HINTS ${DirectX9_LIB_SEARCH_PATH} PATH_SUFFIXES ${DirectX9_LIBPATH_SUFFIX})	

	if (DirectX9_INCLUDE_DIR AND DirectX9_D3D9_LIBRARY)
	  set(DirectX9_D3D9_LIBRARIES ${DirectX9_D3D9_LIBRARIES}
	    ${DirectX9_D3D9_LIBRARY}
	    ${DirectX9_DXGI_LIBRARY}
        ${DirectX9_DXGUID_LIBRARY}
        ${DirectX9_D3DCOMPILER_LIBRARY}        	  
      )	
    endif ()
    if (DirectX9_D3DX9_LIBRARY)
        set(DirectX9_D3D9_LIBRARIES ${DirectX9_D3D9_LIBRARIES} ${DirectX9_D3DX9_LIBRARY})
    endif ()
    if (DirectX9_DXERR_LIBRARY)
        set(DirectX9_D3D9_LIBRARIES ${DirectX9_D3D9_LIBRARIES} ${DirectX9_DXERR_LIBRARY})
    endif ()

	set(DirectX9_LIBRARY 
		${DirectX9_D3D9_LIBRARIES} 
	)
	
	mark_as_advanced(DirectX9_D3D9_LIBRARY 
					 DirectX9_D3DX9_LIBRARY
					 DirectX9_DXERR_LIBRARY 
					 DirectX9_DXGUID_LIBRARY
					 DirectX9_DXGI_LIBRARY 
					 DirectX9_D3DCOMPILER_LIBRARY)	

	findpkg_finish(DirectX9)
	
endif(WIN32)