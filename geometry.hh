#pragma once

#include <GL/glew.h>

#include <vector>

// Generateurs de geometrie procedurale (data brute, sans rendu).
namespace geometry
{
    // Quad sol horizontal centre, de cote `size`.
    void ground(std::vector<GLfloat>& vertices, std::vector<GLfloat>& normals,
                std::vector<GLfloat>& uv, float size);

    // Cylindre a faces planes (flat normals) sur `segments` tranches.
    void cylinder(std::vector<GLfloat>& vertices,
                  std::vector<GLfloat>& normals_flat,
                  std::vector<GLfloat>& uv_buffer_data, GLfloat radius,
                  GLfloat height, int segments);
} // namespace geometry
