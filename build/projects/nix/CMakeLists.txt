j_project(xirang2 src/xirang2)


j_add_source(header_files ${j_project_root}/include/xirang2 HEADER *.*)
j_add_source(source_files ${j_project_root}/src/xirang2 SOURCE *.*)

if(MSVC)
	include_directories(${j_project_root}/include ${j_project_root}/zlib ${j_lib_boost_include_dir})
else()
	include_directories(${j_project_root}/include ${j_lib_boost_include_dir})
endif()

link_directories(${j_lib_boost_default_link_dir})
add_definitions(-DBOOST_USE_STATIC -DXR_COMMON_DLL_EXPORT)
if(MSVC)
	set_rc_info(xirang2 "Xirang2 Library")
endif()

add_library(xirang2 SHARED ${source_files} ${header_files} ${protein_resource_file})
j_set_target_default(xirang2)
add_dependencies(xirang2 zlib)
if(MSVC)
	target_link_libraries(xirang2 LINK_PRIVATE zlib)	#for boost, MSVC use auto link
else()
	target_link_libraries(xirang2 LINK_PRIVATE boost_filesystem${j_boost_postfix} boost_system${j_boost_postfix} pthread zlib)
endif()

set_property(TARGET xirang2 PROPERTY FOLDER Library)
