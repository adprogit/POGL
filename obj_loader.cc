#include "obj_loader.hh"

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <algorithm>
#include <cctype>
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



// Repli heuristique : choisit une couleur cel-shade plausible d'apres le nom
// du materiau / de la texture, utilise quand le baseColorFactor n'est pas
// significatif (souvent blanc quand la vraie couleur est dans la texture).
static void color_from_name(const std::string& raw, float out[3])
{
    std::string n = raw;
    std::transform(n.begin(), n.end(), n.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    auto has = [&](const char* k) { return n.find(k) != std::string::npos; };
    auto set = [&](float r, float g, float b) {
        out[0] = r;
        out[1] = g;
        out[2] = b;
    };

    if (has("bark") || has("trunk") || has("wood") || has("stem")
        || has("log"))
        set(0.42f, 0.26f, 0.15f); // ecorce brune
    else if (has("leaf") || has("leaves") || has("foliage") || has("needle")
             || has("pine") || has("branch") || has("tree") || has("plant")
             || has("fir") || has("spruce"))
        set(0.20f, 0.40f, 0.12f); // feuillage vert
    else if (has("grass") || has("grnd") || has("ground") || has("terrain")
             || has("floor") || has("dirt") || has("soil") || has("moss"))
        set(0.33f, 0.45f, 0.20f); // sol mousse
    else if (has("slope") || has("rock") || has("cliff") || has("stone")
             || has("mountain") || has("cliff"))
        set(0.46f, 0.44f, 0.40f); // roche grise
    else
        set(0.45f, 0.50f, 0.30f); // defaut verdatre
}

bool load_gltf(const std::string& path, std::vector<GltfSubmesh>& out)
{
    out.clear();

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

    // Un submesh par materiau : on accumule toute la geometrie d'un meme
    // materiau dans le meme tampon, transformations de noeud appliquees.
    out.resize(scene->mNumMaterials);

    std::function<void(const aiNode*, const aiMatrix4x4&)> visit =
        [&](const aiNode* node, const aiMatrix4x4& parent) {
            aiMatrix4x4 transform = parent * node->mTransformation;
            aiMatrix4x4 normal_mat = transform;
            normal_mat.Inverse().Transpose();

            for (unsigned int m = 0; m < node->mNumMeshes; m++)
            {
                const aiMesh* mesh = scene->mMeshes[node->mMeshes[m]];
                GltfSubmesh& sm = out[mesh->mMaterialIndex];

                for (unsigned int f = 0; f < mesh->mNumFaces; f++)
                {
                    const aiFace& face = mesh->mFaces[f];
                    for (unsigned int i = 0; i < face.mNumIndices; i++)
                    {
                        unsigned int idx = face.mIndices[i];

                        aiVector3D p = transform * mesh->mVertices[idx];
                        sm.vertices.push_back(p.x);
                        sm.vertices.push_back(p.y);
                        sm.vertices.push_back(p.z);

                        if (mesh->HasNormals())
                        {
                            aiVector3D n = normal_mat * mesh->mNormals[idx];
                            n.Normalize();
                            sm.normals.push_back(n.x);
                            sm.normals.push_back(n.y);
                            sm.normals.push_back(n.z);
                        }
                        else
                        {
                            sm.normals.push_back(0);
                            sm.normals.push_back(1);
                            sm.normals.push_back(0);
                        }

                        if (mesh->HasTextureCoords(0))
                        {
                            const aiVector3D& uv = mesh->mTextureCoords[0][idx];
                            sm.uvs.push_back(uv.x);
                            sm.uvs.push_back(uv.y);
                        }
                        else
                        {
                            sm.uvs.push_back(0);
                            sm.uvs.push_back(0);
                        }
                    }
                }
            }

            for (unsigned int c = 0; c < node->mNumChildren; c++)
                visit(node->mChildren[c], transform);
        };

    visit(scene->mRootNode, aiMatrix4x4());

    // Couleur + nom par materiau : baseColorFactor si pertinent, sinon repli
    // heuristique base sur le nom du materiau et de sa texture.
    for (unsigned int mi = 0; mi < scene->mNumMaterials; mi++)
    {
        const aiMaterial* mat = scene->mMaterials[mi];
        GltfSubmesh& sm = out[mi];

        aiString name;
        mat->Get(AI_MATKEY_NAME, name);
        sm.name = name.C_Str();

        aiString tex;
        std::string tex_name;
        if (mat->GetTexture(aiTextureType_BASE_COLOR, 0, &tex) == AI_SUCCESS
            || mat->GetTexture(aiTextureType_DIFFUSE, 0, &tex) == AI_SUCCESS)
            tex_name = tex.C_Str();

        aiColor4D c(1.f, 1.f, 1.f, 1.f);
        bool has_factor = mat->Get(AI_MATKEY_BASE_COLOR, c) == AI_SUCCESS
            || mat->Get(AI_MATKEY_COLOR_DIFFUSE, c) == AI_SUCCESS;
        bool meaningful =
            has_factor && !(c.r > 0.95f && c.g > 0.95f && c.b > 0.95f);

        if (meaningful)
        {
            sm.color[0] = c.r;
            sm.color[1] = c.g;
            sm.color[2] = c.b;
        }
        else
        {
            color_from_name(sm.name + " " + tex_name, sm.color);
        }
    }

    // Retire les submeshes vides (materiaux non references par la geometrie).
    out.erase(std::remove_if(out.begin(), out.end(),
                             [](const GltfSubmesh& s) {
                                 return s.vertices.empty();
                             }),
              out.end());
    return true;
}