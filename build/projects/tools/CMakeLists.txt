j_project(tools tools)

include_directories(${j_project_root}/include ${Boost_INCLUDE_DIRS})

add_definitions(-DNIX_COMMON_DLL_IMPORT)	
j_add_tools(dummy nix)

