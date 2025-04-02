cmake_minimum_required(VERSION 3.0)
# Notre projet est étiqueté hello
project(recorder)
# Crée des variables avec les fichiers à compiler
set(CXX_STANDARD_REQUIRED ON)
set(CXX_STANDARD C++20)
set(BUILD_TYPE debug)
set(CMAKE_CXX_FLAGS "-std=c++2a -ggdb")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(Scope ../src/ScopeMimicry.cpp)
target_include_directories(Scope PRIVATE ../src)
set(SRCS
    test_scope.cpp
    ../src/ScopeMimicry.cpp
    )

set(HEADERS
    ../src/ScopeMimicry.h
    )
# On indique que l'on veut un exécutable "test_scope" compilé à partir des fichiers décrits par les variables SRCS et HEADERS
# add_executable(test_scope ${SRCS} ${HEADERS})
add_executable(test_scope ${SRCS} ${HEADERS})
target_include_directories(test_scope PRIVATE ../src)
