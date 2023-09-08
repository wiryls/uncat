# use directory name as name
get_filename_component(name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" name ${name})
string(TOLOWER ${name} name)

# sources list
set(source main.cc)
set(header sort.h)

# config executable
add_executable       (${name} ${source} ${header})
source_group         (TREE "${PROJECT_SOURCE_DIR}" FILES ${sources} ${header})
set_target_properties(${name} PROPERTIES FOLDER "${PROJECT_NAME}/examples")
set_target_properties(${name} PROPERTIES CXX_STANDARD 23)
