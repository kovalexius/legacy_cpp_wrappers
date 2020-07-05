SET(DirectX_FOUND 0)

if(NOT TARGET DirectX)
	
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(libPath "$ENV{DXSDK_DIR}Lib/x64")
    else()
        set(libPath "$ENV{DXSDK_DIR}Lib/x86")
    endif()
	
    set(DirectX_INCLUDE_DIRS "$ENV{DXSDK_DIR}Include")
    set(DirectX_LIBRARIES d3dx10 d3d11 d3dx11 dxerr dinput8 dxguid dsound dxgi d3dcompiler)
	set(DirectX_FOUND 1)
	
    foreach(lib ${DirectX_LIBRARIES})

        find_file(libFile_${lib} "${lib}.lib" ${libPath})
		
        if(libFile_${lib})            
            add_library(${lib} SHARED IMPORTED)
            
            set_target_properties(${lib} PROPERTIES IMPORTED_IMPLIB_DEBUG ${libFile_${lib}})
            set_target_properties(${lib} PROPERTIES IMPORTED_IMPLIB_RELEASE ${libFile_${lib}})
        else()

            set(DirectX_FOUND 0)
            break()
        endif()
        
    endforeach()

endif()