#include "framebuffer.hh"

#include "gl_debug.hh"

void SceneFbo::init(int width, int height)
{
    glGenFramebuffers(1, &fbo_);
    TEST_OPENGL_ERROR();
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    TEST_OPENGL_ERROR();
    glGenTextures(1, &color_tex_);
    TEST_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_2D, color_tex_);
    TEST_OPENGL_ERROR();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT,
                 NULL);
    TEST_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    TEST_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    TEST_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    TEST_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    TEST_OPENGL_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           color_tex_, 0);
    TEST_OPENGL_ERROR();
    glGenRenderbuffers(1, &depth_rbo_);
    TEST_OPENGL_ERROR();
    glBindRenderbuffer(GL_RENDERBUFFER, depth_rbo_);
    TEST_OPENGL_ERROR();
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    TEST_OPENGL_ERROR();
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER, depth_rbo_);
    TEST_OPENGL_ERROR();

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "FBO incomplete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
