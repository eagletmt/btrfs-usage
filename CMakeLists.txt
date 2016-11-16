project(btrfs-usage)
cmake_minimum_required(VERSION 2.8)

add_definitions(-Wall -W)

add_executable(btrfs-usage btrfs-usage.c)

install(PROGRAMS ${CMAKE_BINARY_DIR}/btrfs-usage DESTINATION bin)
