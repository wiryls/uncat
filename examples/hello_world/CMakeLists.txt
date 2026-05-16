# use directory name as name
get_filename_component(name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" name ${name})
string(TOLOWER ${name} name)

# sources list
set(source main.cc)

# config executable
add_executable         (${name} ${source} ${header})
source_group           (TREE "${PROJECT_SOURCE_DIR}" FILES ${sources} ${header})
set_target_properties  (${name} PROPERTIES FOLDER "${PROJECT_NAME}/examples")
target_compile_features(${name} PRIVATE cxx_std_23)
target_link_libraries  (${name} PRIVATE uncat::cta uncat::fsm)
