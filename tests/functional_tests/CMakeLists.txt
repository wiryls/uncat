# use dir name as name
get_filename_component(name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" name ${name})
string(TOLOWER ${name} name)

# source lists
set(sources
    orz_test.cpp)

# executable
add_executable         (${name} ${sources})
set_target_properties  (${name} PROPERTIES FOLDER "${PROJECT_NAME}/tests")
target_link_libraries  (${name} PRIVATE Catch2::Catch2WithMain uncat)
target_compile_features(${name} PRIVATE cxx_std_17)

# add for CTest
add_test(NAME ${name} COMMAND ${name})
