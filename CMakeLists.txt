cmake_minimum_required(VERSION 3.21.2)
project(OPENGL)
set(CMAKE_CXX_STANDARD 20)

add_library(mygl SHARED matrix4.cc init.cc shaders.cc obj_loader.cc img/image.cpp img/image_io.cpp)
target_include_directories(mygl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(mygl PRIVATE -Wall -Wextra -Werror -pedantic)

if(APPLE)
    target_include_directories(mygl PUBLIC /opt/homebrew/include)
    find_library(OPENGL_FRAMEWORK OpenGL REQUIRED)
    find_library(GLUT_FRAMEWORK   GLUT   REQUIRED)
    find_library(GLEW_LIB GLEW REQUIRED PATHS /opt/homebrew/lib)
    target_link_libraries(mygl PUBLIC ${GLEW_LIB} ${OPENGL_FRAMEWORK} ${GLUT_FRAMEWORK})
    target_compile_options(mygl PUBLIC -Wno-deprecated-declarations)
else()
    target_link_libraries(mygl PUBLIC GL glut GLEW)
endif()

add_executable(main main.cpp)
target_link_libraries(main PRIVATE mygl)

add_custom_command(TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:main>
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:main>/shaders
    COMMENT "Copying assets and shaders to build directory")
