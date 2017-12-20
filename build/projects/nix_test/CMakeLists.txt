j_project(nix_test test/nix)

j_aux_add_source(source_files)
include_directories(${j_project_root}/include ${Boost_INCLUDE_DIRS})
add_definitions(-DNIX_COMMON_DLL_IMPORT)

add_executable(nix_ut ${source_files} ${protein_resource_file})
j_set_target_default(nix_ut)
target_link_libraries(nix_ut Boost::boost_unit_test_framework Boost::boost_filesystem Boost::boost_system pthread nix)

set_property(TARGET nix_ut PROPERTY FOLDER Test)

add_custom_target(nix_ut_run ALL "${j_default_deploy_path}/$<TARGET_FILE_NAME:nix_ut>" "--run_test=*/*_case")
set_property(TARGET nix_ut_run PROPERTY FOLDER UnitTestRun)
