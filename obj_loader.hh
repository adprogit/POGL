#pragma once
#include <GL/gl.h>
#include <string>
#include <vector>

bool load_obj(const std::string& path, std::vector<GLfloat>& vertices,
              std::vector<GLfloat>& normals, std::vector<GLfloat>& uvs);