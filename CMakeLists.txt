cmake_minimum_required(VERSION 3.21.2)
project(OPENGL)

set(CMAKE_CXX_STANDARD 20)

find_library(GLEW_LIBRARY
    NAMES GLEW
    PATHS /opt/homebrew/lib
    NO_DEFAULT_PATH
    REQUIRED
)

add_library(mygl SHARED matrix4.cc init.cc shaders.cc img/image.cpp img/image_io.cpp)
target_include_directories(mygl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    /opt/homebrew/include
)
target_compile_options(mygl PRIVATE -Wall -Wextra -Werror -pedantic)

if(APPLE)
    find_library(OPENGL_FRAMEWORK OpenGL REQUIRED)
    find_library(GLUT_FRAMEWORK   GLUT   REQUIRED)
    target_link_libraries(mygl PUBLIC ${GLEW_LIBRARY} ${OPENGL_FRAMEWORK} ${GLUT_FRAMEWORK})
    target_compile_options(mygl PUBLIC -Wno-deprecated-declarations)
else()
    find_library(GL_LIBRARY   NAMES GL   PATHS /opt/homebrew/lib NO_DEFAULT_PATH REQUIRED)
    find_library(GLUT_LIBRARY NAMES glut PATHS /opt/homebrew/lib NO_DEFAULT_PATH REQUIRED)
    target_link_libraries(mygl PUBLIC ${GL_LIBRARY} ${GLEW_LIBRARY} ${GLUT_LIBRARY})
endif()

add_executable(main main.cpp)
target_link_libraries(main PRIVATE mygl)

# Copy runtime assets next to the executable after each build, so the binary
# can load them relative to its own directory regardless of the working dir.
add_custom_command(TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:main>
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:main>/shaders
    COMMENT "Copying assets and shaders to build directory")
