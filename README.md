# Some Uncat(egorized) CXX Code

My C++ playground:

- Project organized by modern CMake
- Unit tests with Catch2
- Play with template-meta programming

## Usage

### With CMake `FetchContent`

Dependency:

```cmake
cmake_minimum_required(VERSION 3.14)
include(FetchContent)
```

Integration:

```cmake
FetchContent_Declare(
    uncat
    GIT_REPOSITORY https://github.com/wiryls/uncat
    GIT_TAG        b6f4a1c187b87d52223db9122d22153beebf02c9)

FetchContent_MakeAvailable(uncat)
```

Link:

```cmake
target_link_libraries(${your_target} PRIVATE uncat)
```
