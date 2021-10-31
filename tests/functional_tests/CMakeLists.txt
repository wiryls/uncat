# use dir name as name
get_filename_component(name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" name ${name})
string(TOLOWER ${name} name)

# source lists
set(sources
    exec_test.cpp
    far_test.cpp
    fsm_test.cpp
    messenger_test.cpp
    types_test.cpp)

# executable
add_executable       (${name} ${sources})
set_target_properties(${name} PROPERTIES FOLDER "${PROJECT_NAME}/tests")
set_target_properties(${name} PROPERTIES CXX_STANDARD 23)
target_link_libraries(${name} PRIVATE
    Catch2::Catch2WithMain
    uncat::exec
    uncat::far
    uncat::fsm
    uncat::messenger
    uncat::types)

# add for CTest
add_test(NAME ${name} COMMAND ${name})
