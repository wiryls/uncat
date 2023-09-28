# Some Uncat(egorized) CXX Code

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
    using uncat::operator<<;
    using uncat::binary_tree::node;
    using tree = node<int, 0, node<int, 1, node<int, 3>>, node<int, 2, void, node<int, 4, node<int, 5>>>>;
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
              << uncat::level_order_traversal_t<tree>() << std::endl;
    // Output:
    // 0, 1, 3, 2, 4, 5
    // 3, 1, 0, 2, 5, 4
    // 3, 1, 5, 4, 2, 0
    // 0, 1, 2, 3, 4, 5
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
│  ├─far
│  │  └─include
│  │
│  ├─fsm
│  │  └─include
│  │
│  ├─messenger
│  │  ├─include  // Exported header files.
│  │  └─src      // Some cpp source files and header files.
│  │
│  └─types
│      └─include
│
├─tests          // Tests files (base on Catch2)
│  ├─benchmark_tests
│  └─functional_tests
│
└─third_party    // Dependent libraries (such as Cache2)
```

Executables are placed in `examples` folder. And libraries are in `modules`, test files in `tests`.

e.g. the header-only library `far`:

- Source files: [`modules/far/include/uncat/far/far.h`](modules/far/include/uncat/far/far.h)
- Benchmark tests: [`tests/benchmark_tests/far_test.cc`](tests/benchmark_tests/far_test.cc)
- Functional tests: [`tests/functional_tests/far_test.cc`](tests/functional_tests/far_test.cc)

## Usage

### Build this Project

1. Create a `build` directory for out-of-source building.
    ```shell
    mkdir build
    cd build
    ```
2. Configure it.
    ```shell
    cmake -DUNCAT_BUILD_TESTS=ON -DUNCAT_BUILD_EXAMPLES=ON ..
    ```
3. Build all (in debug mode).
    ```shell
    cmake --build . --config Debug -j 18
    ```

### Import Project in Another Project via CMake `FetchContent`

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
