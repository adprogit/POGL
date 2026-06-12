#include <cmath>
#include <filesystem>
#include <iostream>

#include "camera.hh"
#include "framebuffer.hh"
#include "geometry.hh"
#include "gl_debug.hh"
#include "gui.hh"
#include "helpers.hh"
#include "img/image.hh"
#include "img/image_io.hh"
#include "init.hh"
#include "obj_loader.hh"
#include "render_params.hh"
#include "render_pass.hh"
#include "renderer.hh"
#include "scene.hh"
#include "shaders.hh"

std::vector<TreeInstance> trees;

program g_program;
program sky_program;
program ground_program;
std::vector<GLfloat> g_verts;
std::vector<GLfloat> g_normals;
std::vector<GLfloat> g_uv;

Renderer g_renderer;
program post_program;

RenderParams g_params;
bool g_gui_visible = false;

program trunk_program;
program leaves_program;
std::vector<GLfloat> trunk_v;
std::vector<GLfloat> trunk_n;
std::vector<GLfloat> trunk_uv;
std::vector<GLfloat> leaf_v;
std::vector<GLfloat> leaf_n;
std::vector<GLfloat> leaf_uv;

program grass_program;
std::vector<GLfloat> grass_v;
std::vector<GLfloat> grass_n;
std::vector<GLfloat> grass_uv;
std::vector<TreeInstance> grass;

struct Decor
{
    program prog;
    std::vector<GLfloat> v;
    std::vector<GLfloat> n;
    std::vector<GLfloat> uv;
    std::vector<TreeInstance> items;
};
Decor bush;
Decor fern;
Decor rock;
Decor mush;

GLuint sky_vao;
const int win_w = 1024;
const int win_h = 1024;

Camera g_camera(win_w, win_h);

void toggle_gui()
{
    g_gui_visible = !g_gui_visible;
    g_camera.set_look_enabled(!g_gui_visible);
}

void keyboard_down(unsigned char key, int x, int y)
{
    if (key == '\t')
    {
        toggle_gui();
        return;
    }
    if (g_gui_visible)
    {
        gui_on_keyboard(key, x, y);
        if (gui_wants_keyboard())
        {
            return;
        }
    }
    g_camera.on_key_down(key);
}
void keyboard_up(unsigned char key, int x, int y)
{
    if (g_gui_visible)
    {
        gui_on_keyboard_up(key, x, y);
    }
    // Always release camera keys so none get stuck when the GUI grabs focus.
    g_camera.on_key_up(key);
}
void special_down(int key, int x, int y)
{
    if (g_gui_visible)
    {
        gui_on_special(key, x, y);
    }
}
void special_up(int key, int x, int y)
{
    if (g_gui_visible)
    {
        gui_on_special_up(key, x, y);
    }
}
void mouse_button(int button, int state, int x, int y)
{
    if (g_gui_visible)
    {
        gui_on_mouse(button, state, x, y);
    }
}
void motion(int x, int y)
{
    if (g_gui_visible)
    {
        gui_on_motion(x, y);
    }
    else
    {
        g_camera.on_passive_motion(x, y);
    }
}
void passive_motion(int x, int y)
{
    if (g_gui_visible)
    {
        gui_on_motion(x, y);
    }
    else
    {
        g_camera.on_passive_motion(x, y);
    }
}

void idle()
{
    g_camera.update();
    glutPostRedisplay();
}

void reshape(int w, int h)
{
    if (h == 0)
    {
        h = 1;
    }
    g_camera.set_viewport(w, h);
    g_renderer.resize(w, h);
    gui_reshape(w, h);
}

void display()
{
    glutSetCursor(g_gui_visible ? GLUT_CURSOR_INHERIT : GLUT_CURSOR_NONE);

    mygl::vector3 sun_dir = g_params.sun_dir;
    // Avoid normalizing a null vector if all sliders sit at zero.
    if (std::fabs(sun_dir.a_) + std::fabs(sun_dir.b_) + std::fabs(sun_dir.c_)
        < 1e-4f)
    {
        sun_dir = mygl::vector3(0.0f, 1.0f, 0.0f);
    }
    sun_dir.normalize();

    RenderContext ctx;
    ctx.view = g_camera.view();
    ctx.proj = g_camera.proj();
    ctx.sun_dir = sun_dir;
    ctx.sun_color = g_params.sun_color;
    ctx.cam_pos = g_camera.position();
    ctx.time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    ctx.params = g_params;
    g_renderer.render(ctx);
    gui_render(g_params, g_camera, g_gui_visible);
    glutSwapBuffers();
}

int main(int argc, char* argv[])
{
    if (!init_glut(argc, argv))
    {
        return 1;
    }
    if (!init_glew())
    {
        return 1;
    }
    if (!init_gl())
    {
        return 1;
    }
    g_renderer.init(win_w, win_h);
    gui_init(win_w, win_h);
    trees = make_forest(30, 80.0f);

    const std::filesystem::path base =
        std::filesystem::absolute(argv[0]).parent_path();
    auto asset = [&](const char* rel) { return (base / rel).string(); };

    try
    {
        trunk_program = init_shaders(asset("shaders/vertex.shd"),
                                     asset("shaders/fragment.shd"));
        leaves_program = init_shaders(asset("shaders/vertex.shd"),
                                      asset("shaders/leaves_fragment.shd"));
        grass_program = init_shaders(asset("shaders/vertex.shd"),
                                     asset("shaders/leaves_fragment.shd"));
        g_program = init_shaders(asset("shaders/vertex.shd"),
                                 asset("shaders/fragment.shd"));
        sky_program = init_shaders(asset("shaders/sky_vertex.shd"),
                                   asset("shaders/sky_fragment.shd"));
        ground_program = init_shaders(asset("shaders/groundVertex.shd"),
                                      asset("shaders/groundFragment.shd"));
        post_program = init_shaders(asset("shaders/post_vertex.shd"),
                                    asset("shaders/post_fragment.shd"));
    }
    catch (const std::exception& e)
    {
        std::cerr << "init_shaders threw: " << e.what() << std::endl;
        return 1;
    }

    if (!g_program.is_ready())
    {
        std::cerr << "Shader program not ready:\n"
                  << g_program.get_log() << std::endl;
        return 1;
    }
    glGenVertexArrays(1, &sky_vao);
    if (!load_obj(asset("real_pine_bark.obj").c_str(), trunk_v, trunk_n,
                  trunk_uv))
    {
        std::cerr << "Could not load laf.obj" << std::endl;
        return 1;
    }
    if (!load_obj(asset("real_pine_leaves.obj").c_str(), leaf_v, leaf_n,
                  leaf_uv))
    {
        std::cerr << "Could not load trunk.obj" << std::endl;
        return 1;
    }

    trunk_program.init_object(trunk_v, trunk_n, trunk_uv);
    leaves_program.init_object(leaf_v, leaf_n, leaf_uv);
    g_program.init_POV(mygl::vector3(4.0f, 1.0f, 4.5f),
                       mygl::vector3(0.0f, 0.0f, 0.0f),
                       mygl::vector3(0.0f, 1.0f, 0.0f), -1.0f, 1.0f, -1.0f,
                       1.0f, 1.0f, 250.0f);
    tifo::rgb24_image* texture = tifo::load_image(asset("texture.tga").c_str());
    tifo::rgb24_image* lighting =
        tifo::load_image(asset("lighting.tga").c_str());
    g_program.init_texture(texture, lighting);

    tifo::rgb24_image* bark = tifo::load_image(asset("bark.tga").c_str());
    tifo::rgb24_image* bark_ramp = tifo::generate_toon_ramp(bark, 4);
    tifo::rgb24_image* leaf_ramp =
        tifo::generate_toon_ramp_from_color(51, 88, 0, 4);

    trunk_program.init_single_texture(bark, bark_ramp);
    leaves_program.init_single_texture(leaf_ramp, leaf_ramp);

    if (!load_obj(asset("grass.obj").c_str(), grass_v, grass_n, grass_uv))
    {
        std::cerr << "Could not load grass.obj" << std::endl;
        return 1;
    }
    grass_program.init_object(grass_v, grass_n, grass_uv);
    tifo::rgb24_image* grass_ramp =
        tifo::generate_toon_ramp_from_color(80, 140, 30, 4);
    grass_program.init_single_texture(grass_ramp, grass_ramp);
    grass_program.set_albedo(
        mygl::vector3(80.0f / 255.0f, 140.0f / 255.0f, 30.0f / 255.0f));
    grass = make_scatter(400, 90.0f, 0.6f, 1.1f, 7);

    auto setup_decor = [&](Decor& d, const char* obj, int count, float spread,
                           float smin, float smax, unsigned seed,
                           unsigned char r, unsigned char g, unsigned char b) {
        d.prog = init_shaders(asset("shaders/vertex.shd"),
                              asset("shaders/leaves_fragment.shd"));
        if (!load_obj(asset(obj).c_str(), d.v, d.n, d.uv))
        {
            std::cerr << "Could not load " << obj << std::endl;
            return false;
        }
        d.prog.init_object(d.v, d.n, d.uv);
        tifo::rgb24_image* ramp =
            tifo::generate_toon_ramp_from_color(r, g, b, 4);
        d.prog.init_single_texture(ramp, ramp);
        d.prog.set_albedo(mygl::vector3(r / 255.0f, g / 255.0f, b / 255.0f));
        d.items = make_scatter(count, spread, smin, smax, seed);
        return true;
    };

    if (!setup_decor(bush, "Bush_Common.obj", 22, 80.0f, 0.9f, 1.6f, 11, 46,
                     102, 28)
        || !setup_decor(fern, "Fern_1.obj", 45, 85.0f, 0.6f, 1.1f, 23, 70, 132,
                        48)
        || !setup_decor(rock, "Rock_Medium_1.obj", 16, 80.0f, 0.7f, 1.4f, 31,
                        112, 110, 120)
        || !setup_decor(mush, "Mushroom_Common.obj", 18, 70.0f, 0.5f, 0.9f, 47,
                        168, 78, 58))
    {
        return 1;
    }

    geometry::ground(g_verts, g_normals, g_uv, 200.0f);
    ground_program.init_object(g_verts, g_normals, g_uv);
    TEST_OPENGL_ERROR();

    static SkyPass sky_pass(sky_program, sky_vao);
    static GroundPass ground_pass(ground_program, g_verts);
    static GrassPass grass_pass(grass_program, grass, grass_v);
    static GrassPass bush_pass(bush.prog, bush.items, bush.v);
    static GrassPass fern_pass(fern.prog, fern.items, fern.v);
    static GrassPass rock_pass(rock.prog, rock.items, rock.v);
    static GrassPass mush_pass(mush.prog, mush.items, mush.v);
    static ForestPass forest_pass(trunk_program, leaves_program, trees, trunk_v,
                                  leaf_v);
    static PostPass post_pass(post_program, sky_vao);
    g_renderer.add_scene_pass(&sky_pass);
    g_renderer.add_scene_pass(&ground_pass);
    g_renderer.add_scene_pass(&grass_pass);
    g_renderer.add_scene_pass(&bush_pass);
    g_renderer.add_scene_pass(&fern_pass);
    g_renderer.add_scene_pass(&rock_pass);
    g_renderer.add_scene_pass(&mush_pass);
    g_renderer.add_scene_pass(&forest_pass);
    g_renderer.set_post_pass(&post_pass);

    glutDisplayFunc(display);
    TEST_OPENGL_ERROR();
    glutReshapeFunc(reshape);
    TEST_OPENGL_ERROR();
    glutIdleFunc(idle);
    TEST_OPENGL_ERROR();
    glutKeyboardFunc(keyboard_down);
    TEST_OPENGL_ERROR();
    glutKeyboardUpFunc(keyboard_up);
    TEST_OPENGL_ERROR();
    glutSpecialFunc(special_down);
    TEST_OPENGL_ERROR();
    glutSpecialUpFunc(special_up);
    TEST_OPENGL_ERROR();
    glutMouseFunc(mouse_button);
    TEST_OPENGL_ERROR();
    glutMotionFunc(motion);
    TEST_OPENGL_ERROR();
    glutPassiveMotionFunc(passive_motion);
    TEST_OPENGL_ERROR();
    glutIgnoreKeyRepeat(1);
    TEST_OPENGL_ERROR();
    glutMainLoop();
    TEST_OPENGL_ERROR();
    return 0;
}
