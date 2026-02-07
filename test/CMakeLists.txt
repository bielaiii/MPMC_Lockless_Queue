cmake_minimum_required(VERSION 3.20)

add_executable( UT ${CMAKE_CURRENT_LIST_DIR}/main.cpp)

target_link_libraries(UT PRIVATE MCMP_LOCKLESS_QUEUE)

set_target_properties(UT PROPERTIES 
CXX_STANDARD 23
)


target_compile_options(UT PRIVATE -fsanitize=thread)
target_link_options(UT PRIVATE -fsanitize=thread)