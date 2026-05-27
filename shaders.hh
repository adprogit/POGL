#pragma once

#include <GL/gl.h>
#include <GL/glew.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "init.hh"
#include "matrix4.hh"

class program
{
public:
    program();
    ~program() = default;
    static program make_program(const std::string& vertex_shader_src,
                                const std::string& fragment_shader_src);
    const std::string& get_log();
    bool is_ready();
    void use();
    void init_object();
    GLuint vao_id() const
    {
        return vao_id_;
    }
    GLuint prog_id() const
    {
        return prog_id_;
    }
    void init_POV();

private:
    GLuint prog_id_;
    std::string info_log_program_;
    GLchar* info_log_vertex_shader_;
    GLchar* info_log_fragment_;
    GLint is_ready_;
    GLuint vao_id_;
};

program init_shaders();
