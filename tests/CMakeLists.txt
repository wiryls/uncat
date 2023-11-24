#[[ requirements ]]
cmake_minimum_required(VERSION 3.25)                # for FetchContent_Declare(SYSTEM)
enable_testing()                                    # setup for tests.
set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1) # prevent CTest added targets
                                                    # see: https://stackoverflow.com/questions/44949354
include(FetchContent)
set(FETCHCONTENT_QUIET ON)

#[[ dependencies ]]

# catch2 for unit test
FetchContent_Declare(
    catch
    GIT_REPOSITORY https://github.com/catchorg/Catch2
    GIT_TAG         v3.4.0
    GIT_SHALLOW    ON
    SYSTEM
    FIND_PACKAGE_ARGS NAMES Catch2)
set(CATCH_INSTALL_DOCS   OFF CACHE INTERNAL "ignore catch docs")
set(CATCH_INSTALL_EXTRAS OFF CACHE INTERNAL "ignore catch extras")

# enable all dependencies
FetchContent_MakeAvailable(catch)
set_target_properties(Catch2 Catch2WithMain PROPERTIES FOLDER "third_party/tests")

#[[ subprojects ]]

# add all subprojects
add_subdirectory( benchmark_tests)
add_subdirectory(functional_tests)
