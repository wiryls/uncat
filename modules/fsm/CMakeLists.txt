# use directory name as target name
get_filename_component(name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" name ${name})
string(TOLOWER ${name} name)

# collect all header files
set(headers
    include/uncat/fsm/common.h
    include/uncat/fsm/state_machine.h)

# build as a library
add_library(${name} INTERFACE ${headers})
add_library(${PROJECT_NAME}::${name} ALIAS ${name})

# target properties
source_group              (TREE "${CMAKE_CURRENT_SOURCE_DIR}/include" PREFIX "Header Files" FILES ${headers})
set_target_properties     (${name} PROPERTIES FOLDER "${PROJECT_NAME}/modules")
target_compile_features   (${name} INTERFACE cxx_std_20)
target_include_directories(${name} INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
