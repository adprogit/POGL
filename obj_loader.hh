#pragma once
#include <GL/gl.h>
#include <string>
#include <vector>

bool load_obj(const std::string &path, std::vector<GLfloat> &vertices,
              std::vector<GLfloat> &normals, std::vector<GLfloat> &uvs);

// Un groupe de geometrie partageant un meme materiau dans une scene glTF.
// (Une scene multi-materiaux comme `whole_forest` doit etre decoupee ainsi
// pour que chaque partie -- ecorce, sol, feuillage... -- ait sa propre teinte.)
struct GltfSubmesh
{
    std::vector<GLfloat> vertices;
    std::vector<GLfloat> normals;
    std::vector<GLfloat> uvs;
    std::string name;                       // nom du materiau (debug)
    float color[3] = { 0.7f, 0.7f, 0.7f };  // couleur de base cel-shade (0..1)
};

// Charge une scene glTF en regroupant la geometrie par materiau et en
// appliquant les transformations de chaque noeud. La couleur de chaque submesh
// vient du baseColorFactor du materiau, avec un repli heuristique sur le nom
// (ecorce -> brun, feuillage -> vert, sol -> mousse, etc.).
bool load_gltf(const std::string &path, std::vector<GltfSubmesh> &out);
