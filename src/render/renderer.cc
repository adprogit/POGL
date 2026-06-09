#include "renderer.hh"

#include "gl_debug.hh"

void Renderer::init(int width, int height)
{
    width_ = width;
    height_ = height;
    fbo_.init(width, height);
}

void Renderer::resize(int width, int height)
{
    width_ = width;
    height_ = height;
    fbo_.resize(width, height);
}

void Renderer::render(RenderContext ctx)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    TEST_OPENGL_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_.fbo());
    TEST_OPENGL_ERROR();
    glViewport(0, 0, width_, height_);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    TEST_OPENGL_ERROR();

    for (RenderPass* pass : scene_passes_)
    {
        pass->execute(ctx);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    TEST_OPENGL_ERROR();
    glViewport(0, 0, width_, height_);
    TEST_OPENGL_ERROR();
    glClear(GL_COLOR_BUFFER_BIT);
    TEST_OPENGL_ERROR();

    if (post_pass_)
    {
        ctx.scene_color_tex = fbo_.color_tex();
        ctx.scene_depth_tex = fbo_.depth_tex();
        post_pass_->execute(ctx);
    }

    glutSwapBuffers();
    TEST_OPENGL_ERROR();
}
