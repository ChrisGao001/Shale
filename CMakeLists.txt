project(shale)
cmake_minimum_required(VERSION 2.8)
aux_source_directory(src DIR_SRCS)
add_executable(shale ${DIR_SRCS})

