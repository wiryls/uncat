# use dir name as name
get_filename_component(name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" name ${name})
string(TOLOWER ${name} name)

# source lists
set(sources
    finite_state_machine_test.cpp
    messenger_test.cpp
    parallel_world_test.cpp
    types_test.cpp)

# executable
add_executable       (${name} ${sources})
set_target_properties(${name} PROPERTIES FOLDER "${PROJECT_NAME}/tests")
set_target_properties(${name} PROPERTIES CXX_STANDARD 17)
target_link_libraries(${name} PRIVATE Catch2::Catch2WithMain uncat)

# add for CTest
add_test(NAME ${name} COMMAND ${name})
