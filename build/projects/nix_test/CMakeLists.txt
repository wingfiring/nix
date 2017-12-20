j_project(xirang_test test/xirang2)

j_aux_add_source(source_files)
include_directories(${j_project_root}/include  ${j_lib_boost_include_dir})
link_directories(${j_lib_boost_default_link_dir})
add_definitions(-DBOOST_USE_STATIC -DXR_COMMON_DLL_IMPORT)
if(MSVC)
	set_rc_info(xirang2_ut "Xirang2 Unit Test")
endif()

add_executable(xirang2_ut ${source_files} ${protein_resource_file})
j_set_target_default(xirang2_ut)
if(MSVC)
	target_link_libraries(xirang2_ut xirang2)	#for boost, MSVC use auto link
else()
	target_link_libraries(xirang2_ut boost_unit_test_framework${j_boost_postfix}.a boost_filesystem${j_boost_postfix}.a boost_system${j_boost_postfix}.a pthread xirang2 zlib)
endif()


set_property(TARGET xirang2_ut PROPERTY FOLDER Test)

add_custom_target(xirang2_ut_run ALL "${j_default_deploy_path}/$<TARGET_FILE_NAME:xirang2_ut>" "--run_test=*/*_case")
set_property(TARGET xirang2_ut_run PROPERTY FOLDER UnitTestRun)
