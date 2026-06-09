#pragma once

#include <vector>

#include "framebuffer.hh"
#include "render_pass.hh"

// Orchestre le rendu multi-passes : les passes de scene dessinent dans un FBO
// offscreen, puis la passe de post compose vers le framebuffer par defaut.
// Ajouter un objet = pousser une passe, sans toucher au code d'orchestration.
class Renderer
{
public:
    void init(int width, int height);

    // Adapte le FBO et le viewport a une nouvelle taille (plein ecran/resize).
    void resize(int width, int height);

    // Les passes ne sont pas possedees (elles vivent cote appelant).
    void add_scene_pass(RenderPass* pass) { scene_passes_.push_back(pass); }
    void set_post_pass(RenderPass* pass) { post_pass_ = pass; }

    // Execute une frame complete. ctx.scene_color_tex est rempli en interne.
    void render(RenderContext ctx);

private:
    SceneFbo fbo_;
    int width_ = 0, height_ = 0;
    std::vector<RenderPass*> scene_passes_;
    RenderPass* post_pass_ = nullptr;
};
