# Some Uncat(egorized) CXX Code

![GitHub](https://img.shields.io/github/license/wiryls/uncat?label=license&logo=github)
[![cpp-linter](https://github.com/cpp-linter/cpp-linter-action/actions/workflows/cpp-linter.yml/badge.svg)](https://github.com/wiryls/uncat/actions/workflows/main.yml)

My C++ playground:

- Project organized by modern CMake.
- Unit tests with [Catch2](https://github.com/catchorg/Catch2).
- Play with **template-meta programming**!

With the help of C++ template-meta programming, we can even perform [level-order traversal](modules/cta/include/uncat/cta/binary_tree.h) at compile time:

```cpp
#include <iostream>
#include <uncat/cta/binary_tree.h>

int main()
{
    using uncat::cta::operator<<;
    using uncat::cta::binary_tree::node;
    using tree = node<0, node<1, node<3>>, node<2, void, node<4, node<5>>>>;
    /*     0
     *    / \
     *   1   2
     *  /     \
     * 3       4
     *        /
     *       5
     */

    std::cout << uncat::pre_order_traversal_t<tree>() << '\n'
              << uncat::in_order_traversal_t<tree>() << '\n'
              << uncat::post_order_traversal_t<tree>() << '\n'
              << uncat::level_order_traversal_t<tree>() << '\n';
    // Output:
    // 0 1 3 2 4 5
    // 3 1 0 2 5 4
    // 3 1 5 4 2 0
    // 0 1 2 3 4 5
    return 0;
}
```

## Project Layout

Project layout shows as follows:

```text
├─examples       // Some example code.
│  └─hello_world // Source files of an executable.
│
├─modules        // All libraries are placed here.
│  │
│  ├─cta         // Library folder.
│  │  └─include  // A header only library, thus no "src" folder.
│  │
│  └─fsm
│      └─include
│
├─tests          // Tests files (base on Catch2)
│  ├─benchmark_tests
│  └─functional_tests
│
└─third_party    // Dependent libraries (such as Cache2)
```

Executables are placed in `examples` folder. And libraries are in `modules`, test files in `tests`.

e.g. the header-only library `fsm`:

- Source files: [`modules/fsm/include/uncat/fsm/state_machine.h`](modules/fsm/include/uncat/fsm/state_machine.h)
- Benchmark tests: `tests/benchmark_tests/fsm_test.cc` (to be added)
- Functional tests: [`tests/functional_tests/fsm_test.cc`](tests/functional_tests/fsm_test.cc)

## Usage

### Build this Project

1. Create a `build` directory for out-of-source building.

    ```shell
    mkdir build
    ```

2. Configure it.

    ```shell
    cmake -S . -B build -DUNCAT_BUILD_TESTS=ON -DUNCAT_BUILD_EXAMPLES=ON
    ```

3. Build all (in debug mode).

    ```shell
    cmake --build build --config Debug -j 18
    ```

### Import via CMake `FetchContent`

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
    GIT_TAG        8a07d0b741b08b7b1d9b3037fbae8ed60af895c8)

FetchContent_MakeAvailable(uncat)
```

Link:

```cmake
target_link_libraries(${your_target} PRIVATE uncat::cta)
```
