#pragma once

#include <iostream>
#include <algorithm>
#include <vector>


#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/freeglut.h>

#include <iterator>
#include <numeric>
#include <cmath>

namespace mygl {
class vector3 {
  public:
    vector3(GLfloat a, GLfloat b, GLfloat c): a_(a), b_(b), c_(c) {};
    vector3 operator^(const vector3& v) const;
    void normalize();
    GLfloat a_, b_, c_;
};
class matrix4 {
  public:
    matrix4(std::vector<std::vector<GLfloat>> content
            );
    void operator*=(const matrix4& rhs);
    static matrix4 identity();
    std::vector<std::vector<GLfloat>> content_;
};
};

std::ostream& operator <<(std::ostream &out,const mygl::matrix4 &m);

bool init_glut(int &argc,char* argv []);
