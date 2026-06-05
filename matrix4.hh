#pragma once

#include <GL/glew.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#else
#include <GL/freeglut.h>
#include <GL/gl.h>
#endif

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <iterator>
#include <numeric>
#include <vector>

namespace mygl {


class vector3 {
public:
  vector3(GLfloat a, GLfloat b, GLfloat c) : a_(a), b_(b), c_(c) {}
  vector3 operator^(const vector3 &v) const; // produit vectoriel
  float operator*(const vector3 &v) const;   // produit scalaire
  void normalize();
  GLfloat a_, b_, c_;
};

class matrix4 {
public:
  matrix4 inverse() const;
  matrix4();
  matrix4(GLfloat a, GLfloat b, GLfloat c, GLfloat d, GLfloat e, GLfloat f,
          GLfloat g, GLfloat h, GLfloat i, GLfloat j, GLfloat k, GLfloat l,
          GLfloat m, GLfloat n, GLfloat o, GLfloat p);
  matrix4(std::array<GLfloat, 16> arr);
  void operator*=(const matrix4 &rhs);
  matrix4 operator*(const matrix4 &rhs) const;
  static matrix4 identity();
  GLfloat operator()(size_t row, size_t col) const;
  GLfloat &operator()(size_t row, size_t col);
  const GLfloat *data() const { return content_.data(); }

  friend std::ostream &operator<<(std::ostream &out, const matrix4 &m);

private:
  std::array<GLfloat, 16> content_;
};

    matrix4 rotate_y(float a);

matrix4 look_at(const GLfloat &eyeX, const GLfloat &eyeY, const GLfloat &eyeZ,
                const GLfloat &centerX, const GLfloat &centerY,
                const GLfloat &centerZ, const GLfloat &upX, const GLfloat &upY,
                const GLfloat &upZ);

matrix4 frustum(const GLfloat &left, const GLfloat &right,
                const GLfloat &bottom, const GLfloat &top,
                const GLfloat &nearVal, const GLfloat &farVal);

matrix4 translate(GLfloat tx, GLfloat ty, GLfloat tz);
matrix4 scale(GLfloat sx, GLfloat sy, GLfloat sz);

std::ostream &operator<<(std::ostream &out, const matrix4 &m);

bool init_glut(int &argc, char *argv[]);

} // namespace mygl