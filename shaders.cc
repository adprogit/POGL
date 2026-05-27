#include "shaders.hh"

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

program init_shaders()
{
    std::string vrtx_buffr;
    std::string frgmnt_buffr;
    try
    {
        vrtx_buffr =
            readFile("/home/ad/image/OPENGL/TP/pogl_skel_tp/tp2/vertex.shd");
        frgmnt_buffr =
            readFile("/home/ad/image/OPENGL/TP/pogl_skel_tp/tp2/fragment.shd");
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

void program::init_object()
{
    GLfloat vertices[] = { -0.5f, -0.5f, 0.0f, 0.5f, -0.5f,
                           0.0f,  0.0f,  0.5f, 0.0f };

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
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
                     GL_STATIC_DRAW);
        glVertexAttribPointer(vertex_location, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(vertex_location);
    }

    glBindVertexArray(0);
}

void program::init_POV()
{
    glUseProgram(prog_id_);

    mygl::matrix4 mv =
        mygl::look_at(0.0f, 0.0f, 3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    mygl::matrix4 proj = mygl::frustum(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 100.0f);

    GLint mv_loc = glGetUniformLocation(prog_id_, "model_view_matrix");
    GLint proj_loc = glGetUniformLocation(prog_id_, "projection_matrix");
    GLint col_loc = glGetUniformLocation(prog_id_, "color_");

    if (mv_loc != -1)
        glUniformMatrix4fv(mv_loc, 1, GL_FALSE, mv.data());

    if (proj_loc != -1)
        glUniformMatrix4fv(proj_loc, 1, GL_FALSE, proj.data());

    if (col_loc != -1)
        glUniform3f(col_loc, 1.0f, 0.0f, 0.0f);
}