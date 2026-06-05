#include <cmath>
#include <filesystem>
#include <iostream>

#include "camera.hh"
#include "framebuffer.hh"
#include "geometry.hh"
#include "gl_debug.hh"
#include "helpers.hh"
#include "img/image.hh"
#include "img/image_io.hh"
#include "init.hh"
#include "obj_loader.hh"
#include "render_pass.hh"
#include "renderer.hh"
#include "scene.hh"
#include "shaders.hh"

// --- Etat global de la scene -------------------------------------------------
std::vector<TreeInstance> trees;

program g_program;
program q_program;
program sky_program;
program ground_program;
std::vector<GLfloat> g_verts, g_normals, g_uv;

Renderer g_renderer;
program post_program;

mygl::vector3 g_sun_dir(0.3f, 0.7f, 0.4f);
mygl::vector3 g_sun_color(1.0f, 0.95f, 0.85f);

program trunk_program;
program leaves_program;
std::vector<GLfloat> trunk_v, trunk_n, trunk_uv;
std::vector<GLfloat> leaf_v, leaf_n, leaf_uv;

program grass_program;
std::vector<GLfloat> grass_v, grass_n, grass_uv;
std::vector<TreeInstance> grass;

GLuint sky_vao;
const int win_w = 1024, win_h = 1024;

Camera g_camera(win_w, win_h);

// --- Callbacks GLUT (delegent a la camera) -----------------------------------
void keyboard_down(unsigned char key, int, int) { g_camera.on_key_down(key); }
void keyboard_up(unsigned char key, int, int) { g_camera.on_key_up(key); }
void passive_motion(int x, int y) { g_camera.on_passive_motion(x, y); }

void idle()
{
    g_camera.update();
    glutPostRedisplay();
}

void display()
{
    RenderContext ctx;
    ctx.view = g_camera.view();
    ctx.proj = g_camera.proj();
    ctx.sun_dir = g_sun_dir;
    ctx.sun_color = g_sun_color;
    ctx.cam_pos = g_camera.position();
    g_renderer.render(ctx);
}

int main(int argc, char* argv[])
{
    if (!init_glut(argc, argv))
        return 1;
    if (!init_glew())
        return 1;
    if (!init_gl())
        return 1;
    g_renderer.init(win_w, win_h);
    g_sun_dir.normalize();
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
        q_program = init_shaders(asset("shaders/outlineVertexShader.shd"),
                                 asset("shaders/outlineFragmentShader.shd"));
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
    std::cerr << "q ready: " << q_program.is_ready() << "\n";
    if (!q_program.is_ready())
        std::cerr << q_program.get_log() << "\n";
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
    tifo::rgb24_image* leaf_ramp = tifo::generate_toon_ramp_from_color(51, 88, 0, 4);

    trunk_program.init_single_texture(bark, bark_ramp);
    leaves_program.init_single_texture(leaf_ramp, leaf_ramp);

    // --- Herbe : meme logique que les feuilles (mesh .obj + toon ramp vert) ---
    if (!load_obj(asset("grass.obj").c_str(), grass_v, grass_n, grass_uv))
    {
        std::cerr << "Could not load grass.obj" << std::endl;
        return 1;
    }
    grass_program.init_object(grass_v, grass_n, grass_uv);
    tifo::rgb24_image* grass_ramp =
        tifo::generate_toon_ramp_from_color(80, 140, 30, 4);
    grass_program.init_single_texture(grass_ramp, grass_ramp);
    grass = make_scatter(400, 90.0f, 0.6f, 1.1f, 7);

    geometry::ground(g_verts, g_normals, g_uv, 200.0f);
    ground_program.init_object(g_verts, g_normals, g_uv);
    TEST_OPENGL_ERROR();

    // Assemblage de la pipeline : ordre des passes dans le FBO, puis post.
    static SkyPass sky_pass(sky_program, sky_vao);
    static GroundPass ground_pass(ground_program, g_verts);
    static GrassPass grass_pass(grass_program, grass, grass_v);
    static ForestPass forest_pass(q_program, trunk_program, leaves_program,
                                  trees, trunk_v, leaf_v);
    static PostPass post_pass(post_program, sky_vao);
    g_renderer.add_scene_pass(&sky_pass);
    g_renderer.add_scene_pass(&ground_pass);
    g_renderer.add_scene_pass(&grass_pass);
    g_renderer.add_scene_pass(&forest_pass);
    g_renderer.set_post_pass(&post_pass);

    glutDisplayFunc(display);
    TEST_OPENGL_ERROR();
    glutIdleFunc(idle);
    TEST_OPENGL_ERROR();
    glutKeyboardFunc(keyboard_down);
    TEST_OPENGL_ERROR();
    glutKeyboardUpFunc(keyboard_up);
    TEST_OPENGL_ERROR();
    glutPassiveMotionFunc(passive_motion);
    TEST_OPENGL_ERROR();
    glutIgnoreKeyRepeat(1);
    TEST_OPENGL_ERROR();
    glutMainLoop();
    TEST_OPENGL_ERROR();
    return 0;
}
