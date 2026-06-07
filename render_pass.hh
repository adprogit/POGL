#pragma once

#include <vector>

#include "matrix4.hh"
#include "obj_loader.hh"
#include "scene.hh"
#include "shaders.hh"

// Donnees partagees par toutes les passes pour une frame.
struct RenderContext
{
    mygl::matrix4 view;
    mygl::matrix4 proj;
    mygl::vector3 sun_dir{ 0, 0, 0 };
    mygl::vector3 sun_color{ 0, 0, 0 };
    mygl::vector3 cam_pos{ 0, 0, 0 };
    GLuint scene_color_tex = 0; // rempli par le Renderer avant la passe post
};

// Une etape de rendu. Chaque passe gere son propre etat GL et son draw.
class RenderPass
{
public:
    virtual ~RenderPass() = default;
    virtual void execute(const RenderContext& ctx) = 0;
};

// --- Passes concretes --------------------------------------------------------

// Ciel : triangle plein ecran, depth/cull off, reconstruit les rayons.
class SkyPass : public RenderPass
{
public:
    SkyPass(program& prog, GLuint fullscreen_vao)
        : prog_(prog)
        , vao_(fullscreen_vao)
    {}
    void execute(const RenderContext& ctx) override;

private:
    program& prog_;
    GLuint vao_;
};

// Sol : quad cel-shade.
class GroundPass : public RenderPass
{
public:
    GroundPass(program& prog, const std::vector<GLfloat>& verts)
        : prog_(prog)
        , verts_(verts)
    {}
    void execute(const RenderContext& ctx) override;

private:
    program& prog_;
    const std::vector<GLfloat>& verts_;
};

// Foret : par instance, outline (inverted hull) + tronc + feuilles cel-shade.
class ForestPass : public RenderPass
{
public:
    ForestPass(program& outline, program& trunk, program& leaves,
               const std::vector<TreeInstance>& trees,
               const std::vector<GLfloat>& trunk_v,
               const std::vector<GLfloat>& leaf_v)
        : outline_(outline)
        , trunk_(trunk)
        , leaves_(leaves)
        , trees_(trees)
        , trunk_v_(trunk_v)
        , leaf_v_(leaf_v)
    {}
    void execute(const RenderContext& ctx) override;

private:
    program& outline_;
    program& trunk_;
    program& leaves_;
    const std::vector<TreeInstance>& trees_;
    const std::vector<GLfloat>& trunk_v_;
    const std::vector<GLfloat>& leaf_v_;
};

// Herbe : maillage .obj instancie, cel-shade, double-face (cull off).
class GrassPass : public RenderPass
{
public:
    GrassPass(program& prog, const std::vector<TreeInstance>& blades,
              const std::vector<GLfloat>& mesh_v)
        : prog_(prog)
        , blades_(blades)
        , mesh_v_(mesh_v)
    {}
    void execute(const RenderContext& ctx) override;

private:
    program& prog_;
    const std::vector<TreeInstance>& blades_;
    const std::vector<GLfloat>& mesh_v_;
};

// Modele unique (ex. scene glTF chargee via load_gltf) : un seul mesh
// cel-shade, dessine avec une transform fixe (position/echelle/rotation).
class ModelPass : public RenderPass
{
public:
    ModelPass(program& prog, const std::vector<GLfloat>& verts,
              const mygl::matrix4& model)
        : prog_(prog)
        , verts_(verts)
        , model_(model)
    {}
    void execute(const RenderContext& ctx) override;

private:
    program& prog_;
    const std::vector<GLfloat>& verts_;
    mygl::matrix4 model_;
};

// Scene glTF multi-materiaux (ex. `whole_forest`) : un programme cel-shade par
// submesh, dessine avec une transform commune. La couleur de base de chaque
// submesh est envoyee en uniforme `albedo`.
class GltfScenePass : public RenderPass
{
public:
    GltfScenePass(std::vector<program>& progs,
                  const std::vector<GltfSubmesh>& meshes,
                  const mygl::matrix4& model)
        : progs_(progs)
        , meshes_(meshes)
        , model_(model)
    {}
    void execute(const RenderContext& ctx) override;

private:
    std::vector<program>& progs_;
    const std::vector<GltfSubmesh>& meshes_;
    mygl::matrix4 model_;
};

// Post-processing : echantillonne la couleur de scene vers le framebuffer 0.
class PostPass : public RenderPass
{
public:
    PostPass(program& prog, GLuint fullscreen_vao)
        : prog_(prog)
        , vao_(fullscreen_vao)
    {}
    void execute(const RenderContext& ctx) override;

private:
    program& prog_;
    GLuint vao_;
};
