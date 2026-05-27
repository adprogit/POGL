// main.cc

#include <iostream>

#include "init.hh"
#include "shaders.hh"

program g_program;

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    g_program.use();
    glBindVertexArray(g_program.vao_id());

    GLint col_loc = glGetUniformLocation(g_program.prog_id(), "color_");
    if (col_loc != -1)
        glUniform3f(col_loc, 1.0f, 0.0f, 0.0f);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glutSwapBuffers();
}

int main(int argc, char* argv[])
{
    if (!init_glut(argc, argv))
        return 1;
    if (!init_glew())
        return 1;
    if (!init_gl())
        return 1;

    try
    {
        g_program = init_shaders();
    }
    catch (const std::exception& e)
    {
        std::cerr << "init_shaders threw: " << e.what() << std::endl;
        return 1;
    }

    if (!g_program.is_ready())
    {
        std::cerr << "Shader program not ready:\n"
                  << g_program.get_log() << std::endl;
        return 1;
    }

    g_program.init_object();
    g_program.init_POV();

    glutDisplayFunc(display);
    glutMainLoop();

    return 0;
}