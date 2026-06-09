#pragma once

#include <GL/glew.h>

class SceneFbo
{
public:
    void init(int width, int height);

    void resize(int width, int height);

    GLuint fbo() const
    {
        return fbo_;
    }
    GLuint color_tex() const
    {
        return color_tex_;
    }
    GLuint depth_tex() const
    {
        return depth_tex_;
    }

private:
    GLuint fbo_ = 0;
    GLuint color_tex_ = 0;
    GLuint depth_tex_ = 0;
};
