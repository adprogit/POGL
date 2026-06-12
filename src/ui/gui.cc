#include "gui.hh"

#include "camera.hh"
#include "imgui.h"
#include "imgui_impl_glut.h"
#include "imgui_impl_opengl3.h"
#include "render_params.hh"

void gui_init(int win_w, int win_h)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGLUT_Init();
    ImGui_ImplOpenGL3_Init("#version 410");
    // GLUT fires our reshape callback with the real size before the first
    // frame; this just guarantees a valid DisplaySize until then.
    ImGui_ImplGLUT_ReshapeFunc(win_w, win_h);
}

bool gui_wants_mouse()
{
    return ImGui::GetIO().WantCaptureMouse;
}

bool gui_wants_keyboard()
{
    return ImGui::GetIO().WantCaptureKeyboard;
}

void gui_reshape(int w, int h)
{
    ImGui_ImplGLUT_ReshapeFunc(w, h);
}

void gui_on_mouse(int button, int state, int x, int y)
{
    ImGui_ImplGLUT_MouseFunc(button, state, x, y);
}

void gui_on_motion(int x, int y)
{
    ImGui_ImplGLUT_MotionFunc(x, y);
}

void gui_on_keyboard(unsigned char c, int x, int y)
{
    ImGui_ImplGLUT_KeyboardFunc(c, x, y);
}

void gui_on_keyboard_up(unsigned char c, int x, int y)
{
    ImGui_ImplGLUT_KeyboardUpFunc(c, x, y);
}

void gui_on_special(int key, int x, int y)
{
    ImGui_ImplGLUT_SpecialFunc(key, x, y);
}

void gui_on_special_up(int key, int x, int y)
{
    ImGui_ImplGLUT_SpecialUpFunc(key, x, y);
}

static void draw_panel(RenderParams& p, Camera& camera)
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(400.0f, 680.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("Rendering parameters");
    ImGui::Text("%.1f FPS (%.2f ms)", io.Framerate,
                io.Framerate > 0.0f ? 1000.0f / io.Framerate : 0.0f);
    ImGui::TextDisabled("TAB: back to mouse look");
    ImGui::Separator();

    if (ImGui::CollapsingHeader("Sun", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::SliderFloat3("direction", &p.sun_dir.a_, -1.0f, 1.0f);
        ImGui::ColorEdit3("sun color", &p.sun_color.a_);
    }

    if (ImGui::CollapsingHeader("Sky"))
    {
        ImGui::ColorEdit3("zenith", &p.sky_zenith.a_);
        ImGui::ColorEdit3("horizon", &p.sky_horizon.a_);
    }

    if (ImGui::CollapsingHeader("Clouds"))
    {
        ImGui::Checkbox("enabled##clouds", &p.clouds_enabled);
        ImGui::SliderFloat("height", &p.cloud_height, 10.0f, 200.0f);
        ImGui::SliderFloat("scale", &p.cloud_scale, 0.001f, 0.05f, "%.4f");
        ImGui::SliderFloat("speed u", &p.cloud_speed_u, -0.2f, 0.2f, "%.3f");
        ImGui::SliderFloat("speed v", &p.cloud_speed_v, -0.2f, 0.2f, "%.3f");
        ImGui::SliderFloat("cover low", &p.cloud_cover_low, 0.0f, 1.0f);
        ImGui::SliderFloat("cover high", &p.cloud_cover_high, 0.0f, 1.0f);
        ImGui::SliderFloat("opacity", &p.cloud_opacity, 0.0f, 1.0f);
        ImGui::ColorEdit3("shadow color", &p.cloud_shadow.a_);
        ImGui::ColorEdit3("lit color", &p.cloud_lit.a_);
    }

    if (ImGui::CollapsingHeader("Ground"))
    {
        ImGui::ColorEdit3("base color", &p.ground_color.a_);
        ImGui::ColorEdit3("ambient", &p.ground_ambient.a_);
        ImGui::SliderFloat("light wrap", &p.ground_wrap, 0.0f, 1.0f);
    }

    if (ImGui::CollapsingHeader("Toon rim (trunks)"))
    {
        ImGui::SliderFloat("rim low", &p.rim_low, 0.0f, 1.0f);
        ImGui::SliderFloat("rim high", &p.rim_high, 0.0f, 1.0f);
    }

    if (ImGui::CollapsingHeader("God rays"))
    {
        ImGui::Checkbox("enabled##rays", &p.rays_enabled);
        ImGui::SliderInt("samples", &p.rays_samples, 10, 200);
        ImGui::SliderFloat("density", &p.rays_density, 0.0f, 2.0f);
        ImGui::SliderFloat("decay", &p.rays_decay, 0.8f, 1.0f);
        ImGui::SliderFloat("weight", &p.rays_weight, 0.0f, 2.0f);
        ImGui::SliderFloat("exposure", &p.rays_exposure, 0.0f, 1.0f);
    }

    if (ImGui::CollapsingHeader("Outline"))
    {
        ImGui::Checkbox("enabled##outline", &p.outline_enabled);
        ImGui::ColorEdit3("color##outline", &p.outline_color.a_);
        ImGui::SliderFloat("thickness", &p.outline_thickness, 0.5f, 4.0f);
        ImGui::SliderFloat("depth low", &p.edge_low, 0.0f, 1.0f);
        ImGui::SliderFloat("depth high", &p.edge_high, 0.0f, 1.0f);
        ImGui::SliderFloat("normal low", &p.normal_low, 0.0f, 2.0f);
        ImGui::SliderFloat("normal high", &p.normal_high, 0.0f, 2.0f);
    }

    if (ImGui::CollapsingHeader("Color grading"))
    {
        ImGui::Checkbox("warm/cool grade", &p.grading_enabled);
        ImGui::Checkbox("vignette", &p.vignette_enabled);
        ImGui::ColorEdit3("vignette color", &p.vignette_color.a_);
        ImGui::SliderFloat("gamma", &p.gamma, 0.8f, 3.0f);
    }

    if (ImGui::CollapsingHeader("Camera"))
    {
        ImGui::SliderFloat("move speed", &camera.speed(), 0.5f, 20.0f);
        ImGui::SliderFloat("mouse sensitivity", &camera.mouse_speed(), 0.0005f,
                           0.01f, "%.4f");
    }

    if (ImGui::Button("Reset to defaults"))
    {
        p = RenderParams{};
    }

    ImGui::End();
}

void gui_render(RenderParams& params, Camera& camera, bool visible)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGLUT_NewFrame();
    ImGui::NewFrame();
    if (visible)
    {
        draw_panel(params, camera);
    }
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
