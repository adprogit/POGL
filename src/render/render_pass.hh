#pragma once

#include <vector>

#include "matrix4.hh"
#include "render_params.hh"
#include "scene.hh"
#include "shaders.hh"

struct RenderContext
{
    mygl::matrix4 view;
    mygl::matrix4 proj;
    mygl::vector3 sun_dir{ 0, 0, 0 };
    mygl::vector3 sun_color{ 0, 0, 0 };
    mygl::vector3 cam_pos{ 0, 0, 0 };
    float time = 0.0f;
    GLuint scene_color_tex = 0;
    GLuint scene_depth_tex = 0;
    RenderParams params;
};

class RenderPass
{
public:
    virtual ~RenderPass() = default;
    virtual void execute(const RenderContext& ctx) = 0;
};

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

class ForestPass : public RenderPass
{
public:
    ForestPass(program& trunk, program& leaves,
               const std::vector<TreeInstance>& trees,
               const std::vector<GLfloat>& trunk_v,
               const std::vector<GLfloat>& leaf_v)
        : trunk_(trunk)
        , leaves_(leaves)
        , trees_(trees)
        , trunk_v_(trunk_v)
        , leaf_v_(leaf_v)
    {}
    void execute(const RenderContext& ctx) override;

private:
    program& trunk_;
    program& leaves_;
    const std::vector<TreeInstance>& trees_;
    const std::vector<GLfloat>& trunk_v_;
    const std::vector<GLfloat>& leaf_v_;
};

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
