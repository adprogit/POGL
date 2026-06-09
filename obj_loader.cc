#include "obj_loader.hh"

#include <fstream>
#include <iostream>
#include <sstream>

bool load_obj(const std::string& path, std::vector<GLfloat>& out_verts,
              std::vector<GLfloat>& out_normals, std::vector<GLfloat>& out_uvs)
{
    std::ifstream file(path);
    if (!file)
    {
        std::cerr << "Cannot open " << path << std::endl;
        return false;
    }

    std::vector<float> positions;
    std::vector<float> normals;
    std::vector<float> uvs;

    out_verts.clear();
    out_normals.clear();
    out_uvs.clear();

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream s(line);
        std::string prefix;
        s >> prefix;

        if (prefix == "v")
        {
            float x;
            float y;
            float z;
            s >> x >> y >> z;
            positions.push_back(x);
            positions.push_back(y);
            positions.push_back(z);
        }
        else if (prefix == "vn")
        {
            float x;
            float y;
            float z;
            s >> x >> y >> z;
            normals.push_back(x);
            normals.push_back(y);
            normals.push_back(z);
        }
        else if (prefix == "vt")
        {
            float u;
            float v;
            s >> u >> v;
            uvs.push_back(u);
            uvs.push_back(v);
        }
        else if (prefix == "f")
        {
            std::vector<std::string> tokens;
            std::string token;
            while (s >> token)
            {
                tokens.push_back(token);
            }

            auto push_vertex = [&](const std::string& tok) {
                int pi = 0;
                int ti = 0;
                int ni = 0;
                if (sscanf(tok.c_str(), "%d/%d/%d", &pi, &ti, &ni) == 3)
                {
                }
                else if (sscanf(tok.c_str(), "%d//%d", &pi, &ni) == 2)
                {
                    ti = 0;
                }
                else if (sscanf(tok.c_str(), "%d/%d", &pi, &ti) == 2)
                {
                    ni = 0;
                }
                else
                {
                    sscanf(tok.c_str(), "%d", &pi);
                    ti = 0;
                    ni = 0;
                }
                pi--;
                ti--;
                ni--;

                out_verts.push_back(positions[pi * 3 + 0]);
                out_verts.push_back(positions[pi * 3 + 1]);
                out_verts.push_back(positions[pi * 3 + 2]);

                if (ti >= 0 && (size_t)(ti * 2 + 1) < uvs.size())
                {
                    out_uvs.push_back(uvs[ti * 2 + 0]);
                    out_uvs.push_back(uvs[ti * 2 + 1]);
                }
                else
                {
                    out_uvs.push_back(0);
                    out_uvs.push_back(0);
                }

                if (ni >= 0 && (size_t)(ni * 3 + 2) < normals.size())
                {
                    out_normals.push_back(normals[ni * 3 + 0]);
                    out_normals.push_back(normals[ni * 3 + 1]);
                    out_normals.push_back(normals[ni * 3 + 2]);
                }
                else
                {
                    out_normals.push_back(0);
                    out_normals.push_back(1);
                    out_normals.push_back(0);
                }
            };

            for (size_t i = 1; i + 1 < tokens.size(); i++)
            {
                push_vertex(tokens[0]);
                push_vertex(tokens[i]);
                push_vertex(tokens[i + 1]);
            }
        }
    }
    return true;
}