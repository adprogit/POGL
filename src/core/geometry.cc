#include "geometry.hh"

#include <cmath>

namespace geometry
{
    void ground(std::vector<GLfloat>& vertices, std::vector<GLfloat>& normals,
                std::vector<GLfloat>& uv, float size)
    {
        vertices.clear();
        normals.clear();
        uv.clear();
        float h = size / 2.0f;

        float verts[18] = { -h, 0, -h, h, 0, -h, h,  0, h,
                            -h, 0, -h, h, 0, h,  -h, 0, h };
        for (int i = 0; i < 18; i++)
        {
            vertices.push_back(verts[i]);
        }
        for (int i = 0; i < 6; i++)
        {
            normals.push_back(0);
            normals.push_back(1);
            normals.push_back(0);
        }
        for (int i = 0; i < 6; i++)
        {
            uv.push_back(0);
            uv.push_back(0);
        }
    }

    void cylinder(std::vector<GLfloat>& vertices,
                  std::vector<GLfloat>& normals_flat,
                  std::vector<GLfloat>& uv_buffer_data, GLfloat radius,
                  GLfloat height, int segments)
    {
        vertices.clear();
        normals_flat.clear();
        const double step = 2.0 * M_PI / segments;
        const double halfH = 0.5 * height;

        for (int i = 0; i < segments; i++)
        {
            double theta = i * step;
            double theta1 = (i + 1) * step;
            float u0 = (float)i / segments;
            float u1 = (float)(i + 1) / segments;
            GLfloat c0 = std::cos(theta);
            GLfloat s0 = std::sin(theta);
            GLfloat c1 = std::cos(theta1);
            GLfloat s1 = std::sin(theta1);

            GLfloat x0 = radius * c0;
            GLfloat z0 = radius * s0;
            GLfloat x1 = radius * c1;
            GLfloat z1 = radius * s1;

            vertices.push_back(x0);
            vertices.push_back(-halfH);
            vertices.push_back(z0);
            normals_flat.push_back(c0);
            normals_flat.push_back(0.0f);
            normals_flat.push_back(s0);

            vertices.push_back(x1);
            vertices.push_back(-halfH);
            vertices.push_back(z1);
            normals_flat.push_back(c1);
            normals_flat.push_back(0.0f);
            normals_flat.push_back(s1);

            vertices.push_back(x1);
            vertices.push_back(halfH);
            vertices.push_back(z1);
            normals_flat.push_back(c1);
            normals_flat.push_back(0.0f);
            normals_flat.push_back(s1);

            uv_buffer_data.insert(uv_buffer_data.end(), { u0, 0.0f });
            uv_buffer_data.insert(uv_buffer_data.end(), { u1, 0.0f });
            uv_buffer_data.insert(uv_buffer_data.end(), { u0, 1.0f });

            vertices.push_back(x0);
            vertices.push_back(-halfH);
            vertices.push_back(z0);
            normals_flat.push_back(c0);
            normals_flat.push_back(0.0f);
            normals_flat.push_back(s0);

            vertices.push_back(x1);
            vertices.push_back(halfH);
            vertices.push_back(z1);
            normals_flat.push_back(c1);
            normals_flat.push_back(0.0f);
            normals_flat.push_back(s1);

            vertices.push_back(x0);
            vertices.push_back(halfH);
            vertices.push_back(z0);
            normals_flat.push_back(c0);
            normals_flat.push_back(0.0f);
            normals_flat.push_back(s0);

            uv_buffer_data.insert(uv_buffer_data.end(), { u1, 0.0f });
            uv_buffer_data.insert(uv_buffer_data.end(), { u1, 1.0f });
            uv_buffer_data.insert(uv_buffer_data.end(), { u0, 1.0f });
        }
    }
} // namespace geometry
