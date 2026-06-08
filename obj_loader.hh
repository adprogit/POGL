#pragma once
#include <GL/gl.h>
#include <string>
#include <vector>

bool load_obj(const std::string &path, std::vector<GLfloat> &vertices,
              std::vector<GLfloat> &normals, std::vector<GLfloat> &uvs);

// Charge un fichier glTF (.gltf/.glb) via Assimp. Concatène toutes les meshes
// de la scène en tableaux plats triangulés (verts/normals/uvs entrelacés par
// sommet, comme load_obj). Retourne false si le chargement échoue.
bool load_gltf(const std::string &path, std::vector<GLfloat> &vertices,
               std::vector<GLfloat> &normals, std::vector<GLfloat> &uvs);