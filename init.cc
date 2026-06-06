#include "init.hh"

bool init_glut(int& argc, char* argv[])
{
    glutInit(&argc, argv);
#ifdef __APPLE__
    // Apple's legacy GLUT lacks glutInitContextVersion/Profile; request a
    // core profile (up to GL 4.1 on macOS) via the display-mode flag instead.
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH
                        | GLUT_3_2_CORE_PROFILE);
#else
    glutInitContextVersion(4, 1);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif
    glutInitWindowSize(1024, 1024);
    glutInitWindowPosition(10, 10);
    glutCreateWindow(" Test OpenGL − POGL");
    return true;
}

bool init_glew()
{
    return (glewInit() == GLEW_OK);
}

bool init_gl()
{
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);
    glClearColor(0.4, 0.4, 0.4, 1.0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    return true;
}