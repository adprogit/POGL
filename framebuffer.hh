#pragma once

#include <GL/glew.h>

// FBO offscreen pour le rendu de scene (couleur RGB16F + depth/stencil),
// echantillonne ensuite par la passe de post-processing.
class SceneFbo
{
public:
    // Cree le FBO et ses attachements. A appeler apres init du contexte GL.
    void init(int width, int height);

    GLuint fbo() const { return fbo_; }
    GLuint color_tex() const { return color_tex_; }
    GLuint depth_rbo() const { return depth_rbo_; }

private:
    GLuint fbo_ = 0;
    GLuint color_tex_ = 0;
    GLuint depth_rbo_ = 0;
};
