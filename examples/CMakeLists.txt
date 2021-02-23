# some settings for all subdirectories
add_compile_options(
    "$<$<CXX_COMPILER_ID:MSVC>:/WX;/W4;/utf-8>"
    "$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-g;-pedantic-errors;-Wall;-Wextra;-Wconversion;-Werror>")

# add all examples
add_subdirectory(hello_world)
