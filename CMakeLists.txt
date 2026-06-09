cmake_minimum_required(VERSION 3.21.2)
project(OPENGL)
set(CMAKE_CXX_STANDARD 20)

add_library(mygl SHARED
    src/core/matrix4.cc src/core/geometry.cc src/core/camera.cc
    src/render/framebuffer.cc src/render/render_pass.cc src/render/renderer.cc src/render/shaders.cc
    src/io/helpers.cc src/io/init.cc src/io/obj_loader.cc src/io/img/image.cc src/io/img/image_io.cc)
target_include_directories(mygl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render
    ${CMAKE_CURRENT_SOURCE_DIR}/src/io)
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

add_executable(main src/main.cc)
target_link_libraries(main PRIVATE mygl)

# Track the actual asset/shader files so the copy re-runs whenever any change.
file(GLOB_RECURSE ASSET_FILES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/assets/*)
file(GLOB_RECURSE SHADER_FILES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/shaders/*)

set(COPY_STAMP ${CMAKE_CURRENT_BINARY_DIR}/.assets_shaders_copied)

add_custom_command(
    OUTPUT ${COPY_STAMP}
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:main>
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:main>/shaders
    COMMAND ${CMAKE_COMMAND} -E touch ${COPY_STAMP}
    DEPENDS ${ASSET_FILES} ${SHADER_FILES}
    COMMENT "Copying assets and shaders to build directory")

add_custom_target(copy_assets_shaders ALL DEPENDS ${COPY_STAMP})
add_dependencies(copy_assets_shaders main)
