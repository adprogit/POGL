    #include "obj_loader.hh"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <fstream>
#include <functional>
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

    std::vector<float> positions; // x,y,z packés
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
            float x, y, z;
            s >> x >> y >> z;
            positions.push_back(x);
            positions.push_back(y);
            positions.push_back(z);
        }
        else if (prefix == "vn")
        {
            float x, y, z;
            s >> x >> y >> z;
            normals.push_back(x);
            normals.push_back(y);
            normals.push_back(z);
        }
        else if (prefix == "vt")
        {
            float u, v;
            s >> u >> v;
            uvs.push_back(u);
            uvs.push_back(v);
        }
        else if (prefix == "f")
        {
            std::vector<std::string> tokens;
            std::string token;
            while (s >> token)
                tokens.push_back(token);

            auto push_vertex = [&](const std::string& tok) {
                int pi = 0, ti = 0, ni = 0;
                if (sscanf(tok.c_str(), "%d/%d/%d", &pi, &ti, &ni) == 3)
                {
                    // pos/uv/normal
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

            // triangulation en éventail : 3 sommets = 1 triangle, 4 = 2
            // triangles, etc.
            for (size_t i = 1; i + 1 < tokens.size(); i++)
            {
                push_vertex(tokens[0]);
                push_vertex(tokens[i]);
                push_vertex(tokens[i + 1]);
            }
        }
        // ignore tout le reste (mtllib, usemtl, g, o, s, #...)
    }
    return true;
}

bool load_gltf(const std::string& path, std::vector<GLfloat>& out_verts,
               std::vector<GLfloat>& out_normals, std::vector<GLfloat>& out_uvs)
{
    out_verts.clear();
    out_normals.clear();
    out_uvs.clear();

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(
        path,
        aiProcess_Triangulate | aiProcess_GenSmoothNormals
            | aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs);

    if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
        || !scene->mRootNode)
    {
        std::cerr << "Assimp: cannot load " << path << ": "
                  << importer.GetErrorString() << std::endl;
        return false;
    }

    // Parcours récursif des noeuds pour appliquer les transformations locales.
    std::function<void(const aiNode*, const aiMatrix4x4&)> visit =
        [&](const aiNode* node, const aiMatrix4x4& parent) {
            aiMatrix4x4 transform = parent * node->mTransformation;
            aiMatrix4x4 normal_mat = transform;
            normal_mat.Inverse().Transpose();

            for (unsigned int m = 0; m < node->mNumMeshes; m++)
            {
                const aiMesh* mesh = scene->mMeshes[node->mMeshes[m]];
                for (unsigned int f = 0; f < mesh->mNumFaces; f++)
                {
                    const aiFace& face = mesh->mFaces[f];
                    for (unsigned int i = 0; i < face.mNumIndices; i++)
                    {
                        unsigned int idx = face.mIndices[i];

                        aiVector3D p = transform * mesh->mVertices[idx];
                        out_verts.push_back(p.x);
                        out_verts.push_back(p.y);
                        out_verts.push_back(p.z);

                        if (mesh->HasNormals())
                        {
                            aiVector3D n = normal_mat * mesh->mNormals[idx];
                            n.Normalize();
                            out_normals.push_back(n.x);
                            out_normals.push_back(n.y);
                            out_normals.push_back(n.z);
                        }
                        else
                        {
                            out_normals.push_back(0);
                            out_normals.push_back(1);
                            out_normals.push_back(0);
                        }

                        if (mesh->HasTextureCoords(0))
                        {
                            const aiVector3D& uv = mesh->mTextureCoords[0][idx];
                            out_uvs.push_back(uv.x);
                            out_uvs.push_back(uv.y);
                        }
                        else
                        {
                            out_uvs.push_back(0);
                            out_uvs.push_back(0);
                        }
                    }
                }
            }

            for (unsigned int c = 0; c < node->mNumChildren; c++)
                visit(node->mChildren[c], transform);
        };

    visit(scene->mRootNode, aiMatrix4x4());
    return true;
}