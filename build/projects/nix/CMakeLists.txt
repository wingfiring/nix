j_project(nix src/nix)


j_add_source(header_files ${j_project_root}/include/nix HEADER *.*)
j_add_source(source_files ${j_project_root}/src/nix SOURCE *.*)

include_directories(${j_project_root}/include ${Boost_INCLUDE_DIRS})

add_definitions(-DNIX_COMMON_DLL_EXPORT)

add_library(nix SHARED ${source_files} ${header_files})
j_set_target_default(nix)

find_package(Boost COMPONENTS filesystem system)
target_link_libraries(nix LINK_PRIVATE Boost::filesystem Boost::system pthread zlib)

set_property(TARGET nix PROPERTY FOLDER Library)
