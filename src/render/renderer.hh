#pragma once

#include <vector>

#include "framebuffer.hh"
#include "render_pass.hh"

class Renderer
{
public:
    void init(int width, int height);

    void resize(int width, int height);

    void add_scene_pass(RenderPass* pass)
    {
        scene_passes_.push_back(pass);
    }
    void set_post_pass(RenderPass* pass)
    {
        post_pass_ = pass;
    }

    void render(RenderContext ctx);

private:
    SceneFbo fbo_;
    int width_ = 0;
    int height_ = 0;
    std::vector<RenderPass*> scene_passes_;
    RenderPass* post_pass_ = nullptr;
};
