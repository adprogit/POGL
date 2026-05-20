#include "shaders.hh"

program::program()
{
    prog_id_ = -1;
    vertex_shader_src_ = std::string();
    fragment_shader_src_ = std::string();
}

program program::make_program(std::string& vertex_shader_src,
                              std::string& fragment_shader_src)
{
    program p = program();
    // create shaderrs
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    // setting the sources for the vertex shader

    int l_vrtx = 1;
    const char* c_string_vrtx = vertex_shader_src.c_str();
    const GLchar** string_vrtx = &c_string_vrtx;
    GLsizei count_vrtx = 1;
    const GLint* length_vrtx = &l_vrtx;

    glShaderSource(vertex_shader, count_vrtx, string_vrtx, length_vrtx);

    // setting the sources for the fragment shader

    int l_frmgnt = 1;
    const char* c_string_frgmnt = fragment_shader_src.c_str();
    const GLchar** string_frgmnt = &c_string_frgmnt;
    GLsizei count_frgmnt = 1;
    const GLint* length_frgmnt = &l_frmgnt;

    glShaderSource(fragment_shader, count_frgmnt, string_frgmnt, length_frgmnt);

    // compiling the shaders

    glCompileShader(vertex_shader);
    glCompileShader(fragment_shader);

    // getting log length for the shaders

    GLint vertex_compiled;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &vertex_compiled);
    if (vertex_compiled != GL_TRUE)
    {
        GLint info_log_length_vertex;

        glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH,
                      &info_log_length_vertex);

        GLchar info_log_vertex;

        glGetShaderInfoLog(vertex_shader, info_log_length_vertex, NULL,
                           &info_log_vertex);
        p.info_log_program_ = &info_log_vertex;
        p.is_ready_ = GL_FALSE;
        return p;
    }

    GLint fragment_compiled;
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &fragment_compiled);
    if (fragment_compiled != GL_TRUE)
    {
        GLint info_log_length_fragment;
        glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH,
                      &info_log_length_fragment);
        GLchar info_log_fragment;
        glGetShaderInfoLog(fragment_shader, info_log_length_fragment, NULL,
                           &info_log_fragment);
        p.info_log_program_ = &info_log_fragment;
        p.is_ready_ = GL_FALSE;
        return p;
    }

    // creating the program
    GLuint prog_id = glCreateProgram();

    p.prog_id_ = prog_id;

    // attach the vertex and fragment shader
    glAttachShader(prog_id, fragment_shader);
    glAttachShader(prog_id, vertex_shader);

    // linking the program
    glLinkProgram(prog_id);

    GLint program_linked;
    glGetProgramiv(prog_id, GL_LINK_STATUS, &program_linked);
    if (program_linked != true)
    {
        GLint info_log_length;
        glGetProgramiv(prog_id, GL_INFO_LOG_LENGTH, &info_log_length);
        GLchar info_log_program;
        glGetProgramInfoLog(prog_id, info_log_length, NULL, &info_log_program);
        p.info_log_program_ = &info_log_program;
    }
    else
    {
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
    }
    p.is_ready_ = program_linked;
    return p;
}

char* program::get_log()
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

void init_shaders()
{
    std::ifstream file_vertex("vertex_shader.shd");
    std::ifstream file_fragment("fragment_shader.shd");
    std::vector<std::string> lines;
}
