# Scope test

The `test` folder contains a code to test the Scope library outside of a microcontroller, by running a set of software tests on a regular (desktop/notebook) computer. This conveniently bypasses the need to have a microcontroller board to develop some parts of the Scope library.

Test code is in [test_scope.cpp](test_scope.cpp)

## Test instructions

To run the test suite, you need to compile and run the `test_scope.cpp` code.

First make sure [CMake](https://cmake.org/) is installed and open a terminal *within* the `test` folder, to run the following commands:

1\. Run cmake to **configure** the test code project (based on the provided [CMakeLists.txt](CMakeLists.txt) file): 

```bash
$ cmake .
```
-  → generate a bunch of files
- Remark: this step is only needed once. If you make change to test code, only the following build step needs to be run again

2\. Run cmake to **build** (compile) the project:

```bash
$ cmake --build .
```
- (alternative command: `$ make`)
- → generate the `test_scope` executable (perhaps`test_scope.exe` on Windows?)

3\. **Run the test**:

```bash
$ ./test_scope
```

- → prints the test results on the terminal
