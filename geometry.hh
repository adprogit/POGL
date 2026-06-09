#pragma once

#include <GL/glew.h>
#include <vector>

namespace geometry
{
    void ground(std::vector<GLfloat>& vertices, std::vector<GLfloat>& normals,
                std::vector<GLfloat>& uv, float size);

    void cylinder(std::vector<GLfloat>& vertices,
                  std::vector<GLfloat>& normals_flat,
                  std::vector<GLfloat>& uv_buffer_data, GLfloat radius,
                  GLfloat height, int segments);
} // namespace geometry
