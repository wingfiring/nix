j_project(tools tools)

include_directories(${j_project_root}/include  ${j_lib_boost_include_dir} )
link_directories(${j_lib_boost_default_link_dir})
if (${j_os_name} STREQUAL linux)
	link_directories("${j_lib_TBB_bin_dir}")
endif()
add_definitions(-DBOOST_USE_STATIC -DXR_COMMON_DLL_IMPORT -DXIRANG2_RAINBOW_DLL_IMPORT)	
if(MSVC)
	set_rc_info(xirang2_tools "Xirang2 Tools")
	set (j_tools_rc ${protein_resource_file})
endif()

j_add_tools(gentypever xirang2 zlib)
j_add_tools(lvvfs xirang2 zlib)
j_add_tools(repo_perf xirang2 zlib)

if (NOT WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-register -Wno-overloaded-virtual")
endif()

j_add_tools(xirang_perf xirang2 )

