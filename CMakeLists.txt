cmake_minimum_required(VERSION 3.13)
project(pintOS)

set(CMAKE_CXX_STANDARD 14)

add_executable(pintOS main.cpp src/vm/pagetable.c src/vm/suplpagetable.c src/vm/swapslot.c)