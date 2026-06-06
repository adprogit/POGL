#include "render_pass.hh"

#include "gl_debug.hh"

void SkyPass::execute(const RenderContext& ctx)
{
    glDisable(GL_DEPTH_TEST);
    TEST_OPENGL_ERROR();
    glDepthMask(GL_FALSE);
    TEST_OPENGL_ERROR();
    glDisable(GL_CULL_FACE);
    TEST_OPENGL_ERROR();
    prog_.use();
    mygl::matrix4 inv_vp = (ctx.proj * ctx.view).inverse();
    prog_.mat4vf("inv_view_proj", inv_vp);
    prog_.init_3f("sun_dir", ctx.sun_dir);
    prog_.init_3f("cam_pos", ctx.cam_pos);
    glBindVertexArray(vao_);
    TEST_OPENGL_ERROR();
    glDrawArrays(GL_TRIANGLES, 0, 3);
    TEST_OPENGL_ERROR();
    glEnable(GL_DEPTH_TEST);
    TEST_OPENGL_ERROR();
    glDepthMask(GL_TRUE);
    TEST_OPENGL_ERROR();
    glEnable(GL_CULL_FACE);
    TEST_OPENGL_ERROR();
}

void GroundPass::execute(const RenderContext& ctx)
{
    glCullFace(GL_BACK);
    TEST_OPENGL_ERROR();
    mygl::matrix4 mv_ground = ctx.view * mygl::translate(0, -2, 0);
    glDisable(GL_CULL_FACE);
    TEST_OPENGL_ERROR();
    prog_.use();
    prog_.mat4vf("model_view_matrix", mv_ground);
    prog_.mat4vf("projection_matrix", ctx.proj);
    prog_.init_3f("sun_dir", ctx.sun_dir);
    prog_.init_3f("sun_color", ctx.sun_color);
    glBindVertexArray(prog_.vao_id());
    TEST_OPENGL_ERROR();
    glDrawArrays(GL_TRIANGLES, 0, verts_.size() / 3);
}

void ForestPass::execute(const RenderContext& ctx)
{
    for (const auto& t : trees_)
    {
        mygl::matrix4 model = mygl::translate(t.x, -2.0f, t.z)
                            * mygl::rotate_y(t.rot)
                            * mygl::scale(t.scale, t.scale, t.scale);
        mygl::matrix4 mv = ctx.view * model;

        // --- OUTLINE tronc ---
        glEnable(GL_CULL_FACE);
        TEST_OPENGL_ERROR();
        glCullFace(GL_FRONT);
        TEST_OPENGL_ERROR();
        outline_.use();
        outline_.mat4vf("model_view_matrix", mv);
        outline_.mat4vf("projection_matrix", ctx.proj);
        glUniform1f(glGetUniformLocation(outline_.prog_id(), "outline_thickness"),
                    0.03f);
        glBindVertexArray(trunk_.vao_id());
        glDrawArrays(GL_TRIANGLES, 0, trunk_v_.size() / 3);

        // --- TRONC ---
        glCullFace(GL_BACK);
        TEST_OPENGL_ERROR();

        trunk_.use();
        glActiveTexture(GL_TEXTURE0);
        TEST_OPENGL_ERROR();

        glBindTexture(GL_TEXTURE_2D, trunk_.texture_id());
        TEST_OPENGL_ERROR();

        glActiveTexture(GL_TEXTURE1);
        TEST_OPENGL_ERROR();

        glBindTexture(GL_TEXTURE_2D, trunk_.lighting_id());
        TEST_OPENGL_ERROR();

        trunk_.mat4vf("model_view_matrix", mv);
        TEST_OPENGL_ERROR();

        trunk_.mat4vf("projection_matrix", ctx.proj);
        TEST_OPENGL_ERROR();

        trunk_.mat4vf("view_matrix", ctx.view);
        trunk_.init_3f("sun_dir", ctx.sun_dir);
        trunk_.init_3f("sun_color", ctx.sun_color);
        glBindVertexArray(trunk_.vao_id());
        TEST_OPENGL_ERROR();

        glDrawArrays(GL_TRIANGLES, 0, trunk_v_.size() / 3);

        // --- FEUILLES ---
        glDisable(GL_CULL_FACE);
        TEST_OPENGL_ERROR();

        leaves_.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, leaves_.texture_id());
        TEST_OPENGL_ERROR();

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, leaves_.lighting_id());
        TEST_OPENGL_ERROR();

        leaves_.mat4vf("model_view_matrix", mv);
        leaves_.mat4vf("view_matrix", ctx.view);
        leaves_.mat4vf("projection_matrix", ctx.proj);
        leaves_.init_3f("sun_dir", ctx.sun_dir);
        leaves_.init_3f("sun_color", ctx.sun_color);
        glBindVertexArray(leaves_.vao_id());
        TEST_OPENGL_ERROR();

        glDrawArrays(GL_TRIANGLES, 0, leaf_v_.size() / 3);
        TEST_OPENGL_ERROR();
    }
}

void GrassPass::execute(const RenderContext& ctx)
{
    // Herbe fine : pas de back-face culling.
    glDisable(GL_CULL_FACE);
    TEST_OPENGL_ERROR();

    prog_.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, prog_.texture_id());
    TEST_OPENGL_ERROR();
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, prog_.lighting_id());
    TEST_OPENGL_ERROR();

    glBindVertexArray(prog_.vao_id());
    TEST_OPENGL_ERROR();

    for (const auto& b : blades_)
    {
        mygl::matrix4 model = mygl::translate(b.x, -2.0f, b.z)
                            * mygl::rotate_y(b.rot)
                            * mygl::scale(b.scale, b.scale, b.scale);
        mygl::matrix4 mv = ctx.view * model;

        prog_.mat4vf("model_view_matrix", mv);
        prog_.mat4vf("view_matrix", ctx.view);
        prog_.mat4vf("projection_matrix", ctx.proj);
        prog_.init_3f("sun_dir", ctx.sun_dir);
        prog_.init_3f("sun_color", ctx.sun_color);

        glDrawArrays(GL_TRIANGLES, 0, mesh_v_.size() / 3);
        TEST_OPENGL_ERROR();
    }
}

void PostPass::execute(const RenderContext& ctx)
{
    glDisable(GL_DEPTH_TEST);
    TEST_OPENGL_ERROR();
    glDisable(GL_CULL_FACE);
    TEST_OPENGL_ERROR();
    prog_.use();
    prog_.mat4vf("view", ctx.view);
    prog_.mat4vf("proj", ctx.proj);
    prog_.init_3f("sun_dir", ctx.sun_dir);
    prog_.init_3f("sun_color", ctx.sun_color);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ctx.scene_color_tex);
    TEST_OPENGL_ERROR();
    GLint loc = glGetUniformLocation(prog_.prog_id(), "scene_tex");
    glUniform1i(loc, 0);

    glBindVertexArray(vao_);
    TEST_OPENGL_ERROR();
    glDrawArrays(GL_TRIANGLES, 0, 3);
    TEST_OPENGL_ERROR();

    glEnable(GL_DEPTH_TEST);
    TEST_OPENGL_ERROR();
    glEnable(GL_CULL_FACE);
    TEST_OPENGL_ERROR();
}
