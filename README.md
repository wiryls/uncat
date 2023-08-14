# Some Uncat(egorized) CXX Code

My C++ playground:

- Project organized by modern CMake.
- Unit tests with [Catch2](https://github.com/catchorg/Catch2).
- Play with template-meta programming.

## Project Layout

Project layout shows as follows:

```text
├─examples // Some example code.
│  └─todo
│
├─modules // All libraries are placed here.
│  │
│  ├─exec       // Library folder.
│  │  ├─include // Exported header files.
│  │  └─src     // Some cpp source files and header files.
│  │
│  ├─far
│  │  └─include // A header only library, no "src" folder.
│  │
│  ├─fsm
│  │  └─include
│  │
│  └─types
│      └─include
│
├─tests  // Tests files (base on Catch2)
│  ├─benchmark_tests
│  └─functional_tests
│
└─third_party // Dependent libraries (such as Cache2)
```

Libraries are placed at `modules`, tests are placed at `tests` folder. e.g. the header-only library `far`:

- Source files: [`modules\far\include\uncat\far\far.h`](modules\far\include\uncat\far\far.h)
- Benchmark tests: [`tests\benchmark_tests\far_test.cc`](tests\benchmark_tests\far_test.cc)
- Functional tests: [`tests\functional_tests\far_test.cc`](tests\functional_tests\far_test.cc)

## Usage

### Import Project via CMake `FetchContent`

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
    GIT_TAG        3f583af48ed3ef4c0ec75cec1943f3d3e2f3669e)

FetchContent_MakeAvailable(uncat)
```

Link:

```cmake
target_link_libraries(${your_target} PRIVATE uncat)
```
