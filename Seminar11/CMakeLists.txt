cmake_minimum_required(VERSION 3.20)
project(Seminar11 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "11.0")
    add_compile_options(-fcoroutines)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "14.0")
    add_compile_options(-fcoroutines -stdlib=libc++)
endif()

add_compile_definitions(BOOST_ASIO_NO_DEPRECATED)

if(WIN32)
    add_compile_definitions(_WIN32_WINNT=0x0A00)
endif()


add_executable(task1_echo_server task1.cpp)
target_link_libraries(task1_echo_server
        PRIVATE
        Boost::boost
        Boost::system
        Threads::Threads
)

add_executable(task2_multiplexer task2.cpp)
target_link_libraries(task2_multiplexer
        PRIVATE
        Boost::boost
        Boost::system
        Threads::Threads
)


add_executable(task3_bank_account task3.cpp)
target_link_libraries(task3_bank_account
        PRIVATE
        Boost::boost
        Boost::system
        Threads::Threads
)