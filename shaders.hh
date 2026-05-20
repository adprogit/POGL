#pragma once
#include <fstream>
#include "matrix4.hh"
#include "init.hh"

class program {
    program();
    ~program() = default;
    static program make_program(std::string&  vertex_shader_src,std::string&  fragment_shader_src);
    char* get_log();
    bool is_ready();
    void use();
    private:
        GLuint prog_id_;
        std::string vertex_shader_src_;
        std::string fragment_shader_src_;
        GLchar* info_log_program_;
        GLchar* info_log_vertex_shader_;
        GLchar* info_log_fragment_;
        GLint is_ready_;

};

