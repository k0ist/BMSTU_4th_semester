cmake_minimum_required(VERSION 3.28)
project(Lab4 CUDA)

set(CMAKE_CUDA_STANDARD 20)

add_executable(task1 task1.cu)

set_target_properties(task1 PROPERTIES CUDA_ARCHITECTURES 86)

add_executable(task2 task2.cu)

set_target_properties(task2 PROPERTIES CUDA_ARCHITECTURES 86)
