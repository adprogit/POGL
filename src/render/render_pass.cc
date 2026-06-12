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
    prog_.init_1f("time", ctx.time);
    const RenderParams& p = ctx.params;
    prog_.init_3f("zenith_color", p.sky_zenith);
    prog_.init_3f("horizon_color", p.sky_horizon);
    prog_.init_1i("clouds_enabled", p.clouds_enabled ? 1 : 0);
    prog_.init_1f("cloud_height", p.cloud_height);
    prog_.init_1f("cloud_scale", p.cloud_scale);
    prog_.init_2f("cloud_speed", p.cloud_speed_u, p.cloud_speed_v);
    prog_.init_1f("cloud_cover_low", p.cloud_cover_low);
    prog_.init_1f("cloud_cover_high", p.cloud_cover_high);
    prog_.init_1f("cloud_opacity", p.cloud_opacity);
    prog_.init_3f("cloud_shadow_color", p.cloud_shadow);
    prog_.init_3f("cloud_lit_color", p.cloud_lit);
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
    prog_.init_3f("ground_color", ctx.params.ground_color);
    prog_.init_3f("ground_ambient", ctx.params.ground_ambient);
    prog_.init_1f("ground_wrap", ctx.params.ground_wrap);
    glBindVertexArray(prog_.vao_id());
    TEST_OPENGL_ERROR();
    glDrawArrays(GL_TRIANGLES, 0, verts_.size() / 3);
}

void ForestPass::execute(const RenderContext& ctx)
{
    for (const auto& t : trees_)
    {
        mygl::matrix4 model = mygl::translate(t.x, -2.0f, t.z)
            * mygl::rotate_y(t.rot) * mygl::scale(t.scale, t.scale, t.scale);
        mygl::matrix4 mv = ctx.view * model;

        glEnable(GL_CULL_FACE);
        TEST_OPENGL_ERROR();
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
        trunk_.init_1f("rim_low", ctx.params.rim_low);
        trunk_.init_1f("rim_high", ctx.params.rim_high);
        glBindVertexArray(trunk_.vao_id());
        TEST_OPENGL_ERROR();

        glDrawArrays(GL_TRIANGLES, 0, trunk_v_.size() / 3);

        TEST_OPENGL_ERROR();
        glDisable(GL_CULL_FACE);
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
        leaves_.init_3f("albedo", leaves_.albedo());
        glBindVertexArray(leaves_.vao_id());
        TEST_OPENGL_ERROR();

        glDrawArrays(GL_TRIANGLES, 0, leaf_v_.size() / 3);
        TEST_OPENGL_ERROR();
    }
}

void GrassPass::execute(const RenderContext& ctx)
{
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
            * mygl::rotate_y(b.rot) * mygl::scale(b.scale, b.scale, b.scale);
        mygl::matrix4 mv = ctx.view * model;

        prog_.mat4vf("model_view_matrix", mv);
        prog_.mat4vf("view_matrix", ctx.view);
        prog_.mat4vf("projection_matrix", ctx.proj);
        prog_.init_3f("sun_dir", ctx.sun_dir);
        prog_.init_3f("sun_color", ctx.sun_color);
        prog_.init_3f("albedo", prog_.albedo());

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
    const RenderParams& p = ctx.params;
    prog_.init_1i("rays_enabled", p.rays_enabled ? 1 : 0);
    prog_.init_1i("rays_samples", p.rays_samples);
    prog_.init_1f("rays_density", p.rays_density);
    prog_.init_1f("rays_decay", p.rays_decay);
    prog_.init_1f("rays_weight", p.rays_weight);
    prog_.init_1f("rays_exposure", p.rays_exposure);
    prog_.init_1f("gamma", p.gamma);
    prog_.init_1i("outline_enabled", p.outline_enabled ? 1 : 0);
    prog_.init_3f("outline_color", p.outline_color);
    prog_.init_1f("outline_thickness", p.outline_thickness);
    prog_.init_1f("edge_low", p.edge_low);
    prog_.init_1f("edge_high", p.edge_high);
    prog_.init_1f("normal_low", p.normal_low);
    prog_.init_1f("normal_high", p.normal_high);
    prog_.init_1i("grading_enabled", p.grading_enabled ? 1 : 0);
    prog_.init_1i("vignette_enabled", p.vignette_enabled ? 1 : 0);
    prog_.init_3f("vignette_color", p.vignette_color);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ctx.scene_color_tex);
    TEST_OPENGL_ERROR();
    glUniform1i(glGetUniformLocation(prog_.prog_id(), "scene_tex"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, ctx.scene_depth_tex);
    TEST_OPENGL_ERROR();
    glUniform1i(glGetUniformLocation(prog_.prog_id(), "depth_tex"), 1);

    glBindVertexArray(vao_);
    TEST_OPENGL_ERROR();
    glDrawArrays(GL_TRIANGLES, 0, 3);
    TEST_OPENGL_ERROR();

    glEnable(GL_DEPTH_TEST);
    TEST_OPENGL_ERROR();
    glEnable(GL_CULL_FACE);
    TEST_OPENGL_ERROR();
}
