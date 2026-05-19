cmake_minimum_required(VERSION 3.28)
project(Lab3)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_TOOLCHAIN_FILE "C:/Users/arsen/vcpkg/scripts/buildsystems/vcpkg.cmake")

add_compile_definitions(_WIN32_WINNT=0x0A00)

find_package(Boost REQUIRED COMPONENTS system thread)
find_package(Threads REQUIRED)

if(MSVC)
    string(REGEX REPLACE "/showIncludes" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

add_executable(server_app
        server_src/main.cpp
        server_src/server.hpp
        server_src/server.cpp
)
target_link_libraries(server_app
        PRIVATE
        Boost::boost
        Boost::system
        Boost::thread
        Threads::Threads
)

add_executable(client_app
        client_src/client.cpp
)
target_link_libraries(client_app
        PRIVATE
        Boost::boost
)