#include "shaders.hh"

#include "img/image.hh"
#include "img/image_io.hh"

program::program()
{
    prog_id_ = 0;
    is_ready_ = GL_FALSE;
    info_log_program_ = std::string();
    vao_id_ = 0;
}

program program::make_program(const std::string& vertex_shader_src,
                              const std::string& fragment_shader_src)
{
    program p = program();
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    const char* c_string_vrtx = vertex_shader_src.c_str();
    const GLchar** string_vrtx = &c_string_vrtx;
    GLsizei count_vrtx = 1;

    glShaderSource(vertex_shader, count_vrtx, string_vrtx, nullptr);

    const char* c_string_frgmnt = fragment_shader_src.c_str();
    const GLchar** string_frgmnt = &c_string_frgmnt;
    GLsizei count_frgmnt = 1;

    glShaderSource(fragment_shader, count_frgmnt, string_frgmnt, nullptr);

    glCompileShader(vertex_shader);
    glCompileShader(fragment_shader);

    GLint vertex_compiled;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &vertex_compiled);
    if (vertex_compiled != GL_TRUE)
    {
        GLint info_log_length_vertex;

        glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH,
                      &info_log_length_vertex);
        if (info_log_length_vertex > 0)
        {
            GLchar* info_log_vertex = new GLchar[info_log_length_vertex];

            glGetShaderInfoLog(vertex_shader, info_log_length_vertex, NULL,
                               info_log_vertex);

            std::string tmp(info_log_vertex);
            delete[] info_log_vertex;
            p.info_log_program_ = std::move(tmp);
        }
        p.is_ready_ = GL_FALSE;
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        return p;
    }

    GLint fragment_compiled;
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &fragment_compiled);
    if (fragment_compiled != GL_TRUE)
    {
        GLint info_log_length_fragment;
        glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH,
                      &info_log_length_fragment);
        if (info_log_length_fragment > 0)
        {
            GLchar* info_log_fragment = new GLchar[info_log_length_fragment];
            glGetShaderInfoLog(fragment_shader, info_log_length_fragment, NULL,
                               info_log_fragment);
            std::string tmp(info_log_fragment);
            delete[] info_log_fragment;
            p.info_log_program_ = std::move(tmp);
        }
        p.is_ready_ = GL_FALSE;
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        return p;
    }

    GLuint prog_id = glCreateProgram();

    p.prog_id_ = prog_id;

    glAttachShader(prog_id, fragment_shader);
    glAttachShader(prog_id, vertex_shader);

    glLinkProgram(prog_id);
    GLint program_linked;

    glGetProgramiv(prog_id, GL_LINK_STATUS, &program_linked);
    if (program_linked != GL_TRUE)
    {
        GLint info_log_length;
        glGetProgramiv(prog_id, GL_INFO_LOG_LENGTH, &info_log_length);
        if (info_log_length > 0)
        {
            GLchar* info_log_program = new GLchar[info_log_length];
            glGetProgramInfoLog(prog_id, info_log_length, NULL,
                                info_log_program);
            std::string tmp(info_log_program);
            delete[] info_log_program;
            p.info_log_program_ = std::move(tmp);
        }
        glDeleteProgram(prog_id);
        p.prog_id_ = 0;
    }
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    p.is_ready_ = program_linked;
    return p;
}

const std::string& program::get_log()
{
    return this->info_log_program_;
}

bool program::is_ready()
{
    return this->is_ready_ == GL_TRUE;
}

void program::use()
{
    if (this->is_ready())
        glUseProgram(this->prog_id_);
}

std::string readFile(const std::string& path)
{
    std::ifstream file(path);

    if (!file)
        throw std::runtime_error("Impossible d'ouvrir le fichier : " + path);

    std::ostringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

program init_shaders(const std::string& vertex_shader_path,
                     const std::string& fragment_shader_path)
{
    std::string vrtx_buffr;
    std::string frgmnt_buffr;
    try
    {
        vrtx_buffr = readFile(vertex_shader_path);
        frgmnt_buffr = readFile(fragment_shader_path);
    }
    catch (...)
    {
        throw std::runtime_error("erro while rading the files");
    }

    auto p = program::make_program(vrtx_buffr, frgmnt_buffr);
    if (p.is_ready())
    {
        p.use();
    }
    else
    {
        std::cerr << p.get_log();
    }
    return p;
}

void program::init_object(const std::vector<GLfloat>& vertices,
                          const std::vector<GLfloat>& normals_flat,
                          const std::vector<GLfloat>& uv)
{
    constexpr int max_nb_vbo = 5;
    int index_vbo = 0;
    GLuint vbo_ids[max_nb_vbo];

    glGenVertexArrays(1, &vao_id_);
    glBindVertexArray(vao_id_);

    glGenBuffers(max_nb_vbo, vbo_ids);

    GLint vertex_location = glGetAttribLocation(prog_id_, "position");
    if (vertex_location != -1)
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[index_vbo++]);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat),
                     vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(vertex_location, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(vertex_location);
    }

    GLint normalFlat = glGetAttribLocation(prog_id_, "normalFlat");
    if (normalFlat != -1)
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[index_vbo++]);
        glBufferData(GL_ARRAY_BUFFER, normals_flat.size() * sizeof(GLfloat),
                     normals_flat.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(normalFlat, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(normalFlat);
    }

    GLint uv_location = glGetAttribLocation(prog_id_, "uv");
    if (uv_location != -1)
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[index_vbo++]);
        glBufferData(GL_ARRAY_BUFFER, uv.size() * sizeof(float), uv.data(),
                     GL_STATIC_DRAW);
        glVertexAttribPointer(uv_location, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(uv_location);
    }

    glBindVertexArray(0);
}

void program::init_texture(tifo::rgb24_image* texture,
                           tifo::rgb24_image* lighting)
{
    glUseProgram(prog_id_);

    GLuint texture_id;
    GLuint lighting_id;

    GLint tex_location;
    GLint light_location;
    GLint texture_units, combined_texture_units;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &combined_texture_units);
    glGenTextures(1, &texture_id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture->sx, texture->sy, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, texture->pixels);
    tex_location = glGetUniformLocation(prog_id_, "texture_sampler");
    glUniform1i(tex_location, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // lighting
    glGenTextures(1, &lighting_id);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, lighting_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, lighting->sx, lighting->sy, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, lighting->pixels);
    light_location = glGetUniformLocation(prog_id_, "lighting_sampler");
    glUniform1i(light_location, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void program::init_POV(mygl::vector3 eye, mygl::vector3 center,
                       mygl::vector3 up, GLfloat left, GLfloat right,
                       GLfloat bottom, GLfloat top, GLfloat nearVal,
                       GLfloat farVal)
{
    glUseProgram(prog_id_);
    mygl::matrix4 mv = mygl::look_at(eye.a_, eye.b_, eye.c_, center.a_,
                                     center.b_, center.c_, up.a_, up.b_, up.c_);

    mygl::matrix4 proj =
        mygl::frustum(left, right, bottom, top, nearVal, farVal);

    GLint mv_loc = glGetUniformLocation(prog_id_, "model_view_matrix");
    GLint proj_loc = glGetUniformLocation(prog_id_, "projection_matrix");
    GLint col_loc = glGetUniformLocation(prog_id_, "color_");
    GLint light_col_loc = glGetUniformLocation(prog_id_, "light_color");
    GLint light_pos_loc = glGetUniformLocation(prog_id_, "light_position");

    if (mv_loc != -1)
    {
        glUniformMatrix4fv(mv_loc, 1, GL_FALSE, mv.data());
    }
    if (proj_loc != -1)
    {
        glUniformMatrix4fv(proj_loc, 1, GL_FALSE, proj.data());
    }
    if (col_loc != -1)
    {
        glUniform3f(col_loc, 1.0f, 0.0f, 0.0f);
    }
    if (light_col_loc != -1)
    {
        glUniform3f(light_col_loc, 1.0f, 1.0f, 1.0f);
    }
    if (light_pos_loc != -1)
    {
        glUniform3f(light_pos_loc, 3.0f, 3.0f, 0.0f);
    }
}

void program::init_3f(const std::string& loc, const mygl::vector3& to_init)
{
    GLint to_be_init = glGetUniformLocation(prog_id_, loc.c_str());
    if (to_be_init != -1)
    {
        glUniform3f(to_be_init, to_init.a_, to_init.b_, to_init.c_);
    }
}
void program::mat4vf(const std::string& loc, const mygl::matrix4& m)
{
    GLint to_be_init = glGetUniformLocation(prog_id_, loc.c_str());
    if (to_be_init != -1)
    {
        glUniformMatrix4fv(to_be_init, 1, GL_FALSE, m.data());
    }
}
