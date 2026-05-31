cmake_minimum_required(VERSION 3.21.2)
project(OPENGL)

add_library(mygl SHARED matrix4.cc init.cc shaders.cc img/image.cpp img/image_io.cpp)
target_compile_options(mygl PRIVATE -Wall -Wextra -Werror -pedantic)
set(CMAKE_CXX_STANDARD 20)
add_executable(main main.cpp)
target_link_libraries(main PRIVATE mygl GL glut GLEW
)

# Copy runtime assets (textures) next to the executable after each build,
# so the binary can load them with a relative path from its own directory.
add_custom_command(TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:main>
    COMMENT "Copying assets to build directory")