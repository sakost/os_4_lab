cmake_minimum_required(VERSION 3.17)
project(4_lab C)

set(CMAKE_C_STANDARD 99)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(4_lab_child child.c shrmem.h)
add_executable(4_lab parent.c shrmem.h)
target_link_libraries(4_lab PRIVATE Threads::Threads rt)
target_link_libraries(4_lab_child PRIVATE Threads::Threads rt)

add_dependencies(4_lab 4_lab_child)
