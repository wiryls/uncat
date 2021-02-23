# use dir name as name
get_filename_component(name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" name ${name})
string(TOLOWER ${name} name)

# sources list
set(source main.cpp)

# config executable
add_executable         (${name} ${source})
source_group           (TREE "${PROJECT_SOURCE_DIR}" FILES ${sources})
set_target_properties  (${name} PROPERTIES FOLDER "${PROJECT_NAME}")
target_link_libraries  (${name} PRIVATE uncat)
target_compile_features(${name} PRIVATE cxx_std_17)
