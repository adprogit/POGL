cmake_minimum_required(VERSION 3.21.2)
project(OPENGL)

add_library(mygl SHARED matrix4.cc init.cc shaders.cc)
target_compile_options(mygl PRIVATE -Wall -Wextra -Werror -pedantic -Wold-style-cast)
set(CMAKE_CXX_STANDARD 20)
add_executable(main main.cpp)
target_link_libraries(main PRIVATE mygl GL glut GLEW
)