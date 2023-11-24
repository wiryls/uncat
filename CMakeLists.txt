# cmake requirement
cmake_minimum_required(VERSION 3.25)

# project infomation
project(uncat
    VERSION 0.3.0
    DESCRIPTION "some uncategorized cxx code"
    LANGUAGES CXX
)

# provide options for users
option(UNCAT_BUILD_TESTS    "build all test programs"   OFF)
option(UNCAT_BUILD_EXAMPLES "build all sample programs" OFF)

# set global properties or options
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "")

# add their and our libraries
add_subdirectory(third_party)
add_subdirectory(modules)

# add executable
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND UNCAT_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# add tests
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND UNCAT_BUILD_TESTS)
    add_subdirectory(tests)
endif()
