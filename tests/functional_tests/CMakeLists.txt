# use directory name as name
get_filename_component(name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" name ${name})
string(TOLOWER ${name} name)

# source lists
set(sources
    fsm_test.cc)

# executable
add_executable(${name} ${sources})
set_target_properties  (${name} PROPERTIES FOLDER "${PROJECT_NAME}/tests")
target_compile_features(${name} PRIVATE cxx_std_20)
target_link_libraries  (${name} PRIVATE
    Catch2::Catch2WithMain
    uncat::cta
    uncat::fsm)

# add for CTest
add_test(NAME ${name} COMMAND ${name})
