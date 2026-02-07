cmake_minimum_required(VERSION 3.20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
PROJECT(LOCKLESS_QUEUE)




add_library(MCMP_LOCKLESS_QUEUE INTERFACE)


if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(MCMP_LOCKLESS_QUEUE INTERFACE -stdlib=libc++)
    target_link_options(MCMP_LOCKLESS_QUEUE INTERFACE -stdlib=libc++)   
endif()

target_include_directories(MCMP_LOCKLESS_QUEUE INTERFACE ${CMAKE_SOURCE_DIR}/include)

target_compile_options(MCMP_LOCKLESS_QUEUE INTERFACE
    $<$<CXX_COMPILER_ID:GNU>:
        $<$<CONFIG:Debug>:-Wall -pedantic -g -Wextra -Wshadow -fsanitize=thread -fsanitize=thread>
        $<$<CONFIG:Release>:-O3>
    >
    $<$<CXX_COMPILER_ID:Clang>:
        $<$<CONFIG:Debug>:-g -Wall -Weverything -Wno-c++98-compat -fsanitize=thread -fsanitize=thread>
        $<$<CONFIG:Release>:-O3>
    >
)

add_subdirectory(test)