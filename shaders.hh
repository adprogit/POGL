#pragma once

#include <GL/glew.h>
#ifdef __APPLE__
#    include <OpenGL/gl.h>
#else
#    include <GL/gl.h>
#endif

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "img/image.hh"
#include "img/image_io.hh"
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
    void init_object(const std::vector<GLfloat>& vertices,
                     const std::vector<GLfloat>& normals,
                     const std::vector<GLfloat>& uv);
    GLuint vao_id() const
    {
        return vao_id_;
    }
    GLuint prog_id() const
    {
        return prog_id_;
    }
    void init_POV(mygl::vector3 eye, mygl::vector3 center, mygl::vector3 up,
                  GLfloat left, GLfloat right, GLfloat bottom, GLfloat top,
                  GLfloat nearVal, GLfloat farVal);
    void init_3f(const std::string& loc, const mygl::vector3& v);
    void mat4vf(const std::string& loc, const mygl::matrix4& m);

    void init_texture(tifo::rgb24_image* texture, tifo::rgb24_image* lighting);
    void init_single_texture(tifo::rgb24_image* tex,
                             tifo::rgb24_image* lighting);
    GLuint texture_id() const
    {
        return texture_id_;
    }
    GLuint lighting_id() const
    {
        return lighting_id_;
    }

    void set_albedo(const mygl::vector3& a)
    {
        albedo_ = a;
    }
    const mygl::vector3& albedo() const
    {
        return albedo_;
    }

private:
    GLuint prog_id_;
    std::string info_log_program_;
    GLint is_ready_;
    GLuint vao_id_;
    GLuint texture_id_ = 0;
    GLuint lighting_id_ = 0;
    mygl::vector3 albedo_ = mygl::vector3(51.0f / 255.0f, 88.0f / 255.0f, 0.0f);
};

program init_shaders(const std::string& vertex_shader_path,
                     const std::string& fragment_shader_path);
