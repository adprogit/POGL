#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <iostream>

#include "camera.hh"
#include "framebuffer.hh"
#include "gl_debug.hh"
#include "helpers.hh"
#include "img/image.hh"
#include "img/image_io.hh"
#include "init.hh"
#include "obj_loader.hh"
#include "render_pass.hh"
#include "renderer.hh"
#include "shaders.hh"

// --- Etat global de la scene -------------------------------------------------
Renderer g_renderer;

program sky_program;
program post_program;

// Scene glTF "whole_forest" : un submesh (et un programme cel-shade) par
// materiau, charge via load_gltf.
std::vector<GltfSubmesh> forest;
std::vector<program> forest_progs;

mygl::vector3 g_sun_dir(0.3f, 0.7f, 0.4f);
mygl::vector3 g_sun_color(1.0f, 0.95f, 0.85f);

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

    const std::filesystem::path base =
        std::filesystem::absolute(argv[0]).parent_path();
    auto asset = [&](const char* rel) { return (base / rel).string(); };

    try
    {
        sky_program = init_shaders(asset("shaders/sky_vertex.shd"),
                                   asset("shaders/sky_fragment.shd"));
        post_program = init_shaders(asset("shaders/post_vertex.shd"),
                                    asset("shaders/post_fragment.shd"));
    }
    catch (const std::exception& e)
    {
        std::cerr << "init_shaders threw: " << e.what() << std::endl;
        return 1;
    }

    glGenVertexArrays(1, &sky_vao);

    // --- Chargement de la scene glTF "whole_forest" -------------------------
    // load_gltf regroupe la geometrie par materiau ; chaque submesh recoit son
    // propre programme cel-shade avec une rampe toon derivee de sa couleur.
    if (!load_gltf(asset("whole_forest/scene.gltf").c_str(), forest))
    {
        std::cerr << "Could not load whole_forest/scene.gltf" << std::endl;
        return 1;
    }

    forest_progs.reserve(forest.size());
    for (const auto& m : forest)
    {
        program p = init_shaders(asset("shaders/vertex.shd"),
                                 asset("shaders/forest_fragment.shd"));
        p.init_object(m.vertices, m.normals, m.uvs);
        tifo::rgb24_image* ramp = tifo::generate_toon_ramp_from_color(
            (uint8_t)(m.color[0] * 255.0f), (uint8_t)(m.color[1] * 255.0f),
            (uint8_t)(m.color[2] * 255.0f), 4);
        p.init_single_texture(ramp, ramp);
        forest_progs.push_back(std::move(p));

        std::cerr << "submesh '" << m.name << "' : " << m.vertices.size() / 3
                  << " verts, color (" << m.color[0] << ", " << m.color[1]
                  << ", " << m.color[2] << ")\n";
    }
    TEST_OPENGL_ERROR();

    // Auto-fit : recentre la scene en XZ, pose sa base au niveau du sol et
    // l'ajuste a une emprise horizontale raisonnable (robuste aux unites glTF).
    float lo[3] = { 1e30f, 1e30f, 1e30f };
    float hi[3] = { -1e30f, -1e30f, -1e30f };
    for (const auto& m : forest)
        for (size_t i = 0; i + 2 < m.vertices.size(); i += 3)
            for (int a = 0; a < 3; a++)
            {
                lo[a] = std::min(lo[a], m.vertices[i + a]);
                hi[a] = std::max(hi[a], m.vertices[i + a]);
            }
    float cx = 0.5f * (lo[0] + hi[0]);
    float cz = 0.5f * (lo[2] + hi[2]);
    float span = std::max(hi[0] - lo[0], hi[2] - lo[2]);
    float s = span > 0.0f ? 150.0f / span : 1.0f;
    mygl::matrix4 forest_xform =
        mygl::translate(-cx * s, -lo[1] * s - 2.0f, -cz * s)
        * mygl::scale(s, s, s);

    // --- Assemblage de la pipeline ------------------------------------------
    static SkyPass sky_pass(sky_program, sky_vao);
    static GltfScenePass forest_pass(forest_progs, forest, forest_xform);
    static PostPass post_pass(post_program, sky_vao);
    g_renderer.add_scene_pass(&sky_pass);
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
