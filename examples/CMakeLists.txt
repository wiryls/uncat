# some settings for all subdirectories
add_compile_options(
    # for gnu or clang
    "$<$<CXX_COMPILER_ID:GNU,Clang>:-pedantic-errors;-Wall;-Wextra;-Wconversion;-Werror>"
    "$<$<AND:$<CXX_COMPILER_ID:GNU,Clang>,$<CONFIG:RELEASE>>:-O3>"
    "$<$<AND:$<CXX_COMPILER_ID:GNU,Clang>,$<CONFIG:DEBUG>>:-g>"
    # for msvc
    "$<$<CXX_COMPILER_ID:MSVC>:/WX;/W4;/utf-8>"
    "$<$<AND:$<CONFIG:RELEASE>,$<CXX_COMPILER_ID:MSVC>>:/O2>")

# add all examples
add_subdirectory(hello_world)
