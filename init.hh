#pragma once


#include <GL/glew.h>
#ifdef __APPLE__
#  include <GLUT/glut.h>
#  include <OpenGL/gl.h>
#else
#  include <GL/freeglut.h>
#  include <GL/gl.h>
#endif


bool init_glut(int& argc, char* argv[]);
bool init_glew();
bool init_gl();
