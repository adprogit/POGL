#include <cmath>
#include <filesystem>
#include <iostream>

#include "img/image.hh"
#include "img/image_io.hh"
#include "init.hh"
#include "obj_loader.hh"
#include "shaders.hh"

program g_program;
program q_program;
program sky_program;
program ground_program;
std::vector<GLfloat> g_verts, g_normals, g_uv;

GLuint scene_fbo;
GLuint scene_color_tex;
GLuint scene_depth_rbo;
program post_program;

mygl::vector3 g_sun_dir(0.3f, 0.7f, 0.4f);
mygl::vector3 g_sun_color(1.0f, 0.95f, 0.85f);

program trunk_program;
program leaves_program;
std::vector<GLfloat> trunk_v, trunk_n, trunk_uv;
std::vector<GLfloat> leaf_v, leaf_n, leaf_uv;

#define TEST_OPENGL_ERROR()                                     \
    do {                                                        \
       GLenum err = glGetError();                               \
       if (err != GL_NO_ERROR)                                  \
        std::cerr << "OpenGL ERROR!" << __LINE__ << std::endl;  \
} while (0)

void ground(std::vector<GLfloat>& vertices, std::vector<GLfloat>& normals,
            std::vector<GLfloat>& uv, float size)
{
    vertices.clear();
    normals.clear();
    uv.clear();
    float h = size / 2.0f;

    float verts[18] = { -h, 0, -h, h, 0, -h, h,  0, h,
                        -h, 0, -h, h, 0, h,  -h, 0, h };
    for (int i = 0; i < 18; i++)
        vertices.push_back(verts[i]);
    for (int i = 0; i < 6; i++)
    {
        normals.push_back(0);
        normals.push_back(1);
        normals.push_back(0);
    }
    for (int i = 0; i < 6; i++)
    {
        uv.push_back(0);
        uv.push_back(0);
    }
}

GLuint sky_vao;

float pos_x = 0.0f, pos_y = 1.0f, pos_z = 5.0f;
float horizontal_angle = 3.14f;
float vertical_angle = 0.0f;
float speed = 3.0f;
float mouse_speed = 0.003f;
const int win_w = 1024, win_h = 1024;

int last_mouse_x = -1, last_mouse_y = -1;

bool key_z = false, key_s = false, key_q = false, key_d = false;

mygl::matrix4 g_view, g_proj;

bool key_up = false, key_down = false, key_left = false, key_right = false;

void cylinder(std::vector<GLfloat>& vertices,
              std::vector<GLfloat>& normals_flat,
              std::vector<GLfloat>& uv_buffer_data, GLfloat radius,
              GLfloat height, int segments)
{
    vertices.clear();
    normals_flat.clear();
    const double step = 2.0 * M_PI / segments;
    const double halfH = 0.5 * height;

    for (int i = 0; i < segments; i++)
    {
        double theta = i * step;
        double theta1 = (i + 1) * step;
        float u0 = (float)i / segments;
        float u1 = (float)(i + 1) / segments;
        GLfloat c0 = std::cos(theta);
        GLfloat s0 = std::sin(theta);
        GLfloat c1 = std::cos(theta1);
        GLfloat s1 = std::sin(theta1);

        GLfloat x0 = radius * c0, z0 = radius * s0;
        GLfloat x1 = radius * c1, z1 = radius * s1;

        vertices.push_back(x0);
        vertices.push_back(-halfH);
        vertices.push_back(z0);
        normals_flat.push_back(c0);
        normals_flat.push_back(0.0f);
        normals_flat.push_back(s0);

        vertices.push_back(x1);
        vertices.push_back(-halfH);
        vertices.push_back(z1);
        normals_flat.push_back(c1);
        normals_flat.push_back(0.0f);
        normals_flat.push_back(s1);

        vertices.push_back(x1);
        vertices.push_back(halfH);
        vertices.push_back(z1);
        normals_flat.push_back(c1);
        normals_flat.push_back(0.0f);
        normals_flat.push_back(s1);

        uv_buffer_data.insert(uv_buffer_data.end(), { u0, 0.0f });
        uv_buffer_data.insert(uv_buffer_data.end(), { u1, 0.0f });
        uv_buffer_data.insert(uv_buffer_data.end(), { u0, 1.0f });

        vertices.push_back(x0);
        vertices.push_back(-halfH);
        vertices.push_back(z0);
        normals_flat.push_back(c0);
        normals_flat.push_back(0.0f);
        normals_flat.push_back(s0);

        vertices.push_back(x1);
        vertices.push_back(halfH);
        vertices.push_back(z1);
        normals_flat.push_back(c1);
        normals_flat.push_back(0.0f);
        normals_flat.push_back(s1);

        vertices.push_back(x0);
        vertices.push_back(halfH);
        vertices.push_back(z0);
        normals_flat.push_back(c0);
        normals_flat.push_back(0.0f);
        normals_flat.push_back(s0);

        uv_buffer_data.insert(uv_buffer_data.end(), { u1, 0.0f });
        uv_buffer_data.insert(uv_buffer_data.end(), { u1, 1.0f });
        uv_buffer_data.insert(uv_buffer_data.end(), { u0, 1.0f });
    }
}
std::vector<GLfloat> vertices;
std::vector<GLfloat> normals_flat;
std::vector<GLfloat> uv;

void keyboard_down(unsigned char key, int, int)
{
    if (key == 'z' || key == 'Z' || key == 'w' || key == 'W')
        key_z = true;
    if (key == 's' || key == 'S')
        key_s = true;
    if (key == 'q' || key == 'Q' || key == 'a' || key == 'A')
        key_q = true;
    if (key == 'd' || key == 'D')
        key_d = true;
    if (key == 27)
        exit(0);
}

void keyboard_up(unsigned char key, int, int)
{
    if (key == 'z' || key == 'Z' || key == 'w' || key == 'W')
        key_z = false;
    if (key == 's' || key == 'S')
        key_s = false;
    if (key == 'q' || key == 'Q' || key == 'a' || key == 'A')
        key_q = false;
    if (key == 'd' || key == 'D')
        key_d = false;
}
void special_down(int key, int, int)
{
    if (key == GLUT_KEY_UP)
        key_up = true;
    if (key == GLUT_KEY_DOWN)
        key_down = true;
    if (key == GLUT_KEY_LEFT)
        key_left = true;
    if (key == GLUT_KEY_RIGHT)
        key_right = true;
}

void special_up(int key, int, int)
{
    if (key == GLUT_KEY_UP)
        key_up = false;
    if (key == GLUT_KEY_DOWN)
        key_down = false;
    if (key == GLUT_KEY_LEFT)
        key_left = false;
    if (key == GLUT_KEY_RIGHT)
        key_right = false;
}

void keyboard(unsigned char key, int, int)
{
    if (key == 27)
        exit(0);
}

int mouse_x = win_w / 2;
int mouse_y = win_h / 2;

void passive_motion(int x, int y)
{
    mouse_x = x;
    mouse_y = y;
}
void compute_matrices_from_inputs()
{
    static int last_time = glutGet(GLUT_ELAPSED_TIME);
    int now = glutGet(GLUT_ELAPSED_TIME);
    float delta_time = (now - last_time) / 1000.0f;
    last_time = now;

    if (last_mouse_x >= 0)
    {
        int dx = mouse_x - last_mouse_x;
        int dy = mouse_y - last_mouse_y;
        if (std::abs(dx) < 200 && std::abs(dy) < 200)
        {
            horizontal_angle -= dx * mouse_speed;
            vertical_angle -= dy * mouse_speed;
        }
    }
    last_mouse_x = mouse_x;
    last_mouse_y = mouse_y;

    const int margin = 80;
    const float edge_speed = 1.5f;
    if (mouse_x < margin)
        horizontal_angle +=
            edge_speed * delta_time * (margin - mouse_x) / float(margin);
    if (mouse_x > win_w - margin)
        horizontal_angle -= edge_speed * delta_time
            * (mouse_x - (win_w - margin)) / float(margin);
    if (mouse_y < margin)
        vertical_angle +=
            edge_speed * delta_time * (margin - mouse_y) / float(margin);
    if (mouse_y > win_h - margin)
        vertical_angle -= edge_speed * delta_time * (mouse_y - (win_h - margin))
            / float(margin);

    if (vertical_angle > 1.5f)
        vertical_angle = 1.5f;
    if (vertical_angle < -1.5f)
        vertical_angle = -1.5f;

    float fx = std::cos(vertical_angle) * std::sin(horizontal_angle);
    float fy = std::sin(vertical_angle);
    float fz = std::cos(vertical_angle) * std::cos(horizontal_angle);

    float rx = std::sin(horizontal_angle - 1.5708f);
    float rz = std::cos(horizontal_angle - 1.5708f);

    if (key_z)
    {
        pos_x += fx * delta_time * speed;
        pos_y += fy * delta_time * speed;
        pos_z += fz * delta_time * speed;
    }
    if (key_s)
    {
        pos_x -= fx * delta_time * speed;
        pos_y -= fy * delta_time * speed;
        pos_z -= fz * delta_time * speed;
    }
    if (key_d)
    {
        pos_x += rx * delta_time * speed;
        pos_z += rz * delta_time * speed;
    }
    if (key_q)
    {
        pos_x -= rx * delta_time * speed;
        pos_z -= rz * delta_time * speed;
    }

    g_view = mygl::look_at(pos_x, pos_y, pos_z, pos_x + fx, pos_y + fy,
                           pos_z + fz, 0, 1, 0);
    g_proj = mygl::frustum(-1, 1, -1, 1, 1.0f, 250.0f);
}

void idle()
{
    compute_matrices_from_inputs();
    glutPostRedisplay();
}
void init_fbo()
{
    glGenFramebuffers(1, &scene_fbo);
    TEST_OPENGL_ERROR();
    glBindFramebuffer(GL_FRAMEBUFFER, scene_fbo);
    TEST_OPENGL_ERROR();
    glGenTextures(1, &scene_color_tex);
    TEST_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_2D, scene_color_tex);
    TEST_OPENGL_ERROR();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 1024, 1024, 0, GL_RGB, GL_FLOAT,
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
                           scene_color_tex, 0);
    TEST_OPENGL_ERROR();
    glGenRenderbuffers(1, &scene_depth_rbo);
    TEST_OPENGL_ERROR();
    glBindRenderbuffer(GL_RENDERBUFFER, scene_depth_rbo);
    TEST_OPENGL_ERROR();
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1024, 1024);
    TEST_OPENGL_ERROR();
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER, scene_depth_rbo);
    TEST_OPENGL_ERROR();

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "FBO incomplete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    TEST_OPENGL_ERROR();
    glBindFramebuffer(GL_FRAMEBUFFER, scene_fbo);
    TEST_OPENGL_ERROR();
    glViewport(0, 0, 1024, 1024);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    TEST_OPENGL_ERROR();

    glDisable(GL_DEPTH_TEST);
    TEST_OPENGL_ERROR();
    glDepthMask(GL_FALSE);
    TEST_OPENGL_ERROR();
    glDisable(GL_CULL_FACE);
    TEST_OPENGL_ERROR();
    sky_program.use();
    mygl::matrix4 inv_vp = (g_proj * g_view).inverse();
    sky_program.mat4vf("inv_view_proj", inv_vp);
    sky_program.init_3f("sun_dir", g_sun_dir);
    sky_program.init_3f("cam_pos", mygl::vector3(pos_x, pos_y, pos_z));
    glBindVertexArray(sky_vao);
    TEST_OPENGL_ERROR();
    glDrawArrays(GL_TRIANGLES, 0, 3);
    TEST_OPENGL_ERROR();
    glEnable(GL_DEPTH_TEST);
    TEST_OPENGL_ERROR();
    glDepthMask(GL_TRUE);
    TEST_OPENGL_ERROR();
    glEnable(GL_CULL_FACE);
    TEST_OPENGL_ERROR();

    glCullFace(GL_BACK);
    TEST_OPENGL_ERROR();
    mygl::matrix4 mv_ground = g_view * mygl::translate(0, -2, 0);
    glDisable(GL_CULL_FACE);
    TEST_OPENGL_ERROR();
    ground_program.use();
    ground_program.mat4vf("model_view_matrix", mv_ground);
    ground_program.mat4vf("projection_matrix", g_proj);
    ground_program.init_3f("sun_dir", g_sun_dir);
    ground_program.init_3f("sun_color", g_sun_color);
    glBindVertexArray(ground_program.vao_id());
    TEST_OPENGL_ERROR();
    glDrawArrays(GL_TRIANGLES, 0, g_verts.size() / 3);

    mygl::matrix4 mv = g_view * mygl::translate(0, -2, 0);

    // --- TRONC ---
    glEnable(GL_CULL_FACE);
    TEST_OPENGL_ERROR();
    glCullFace(GL_BACK);
    TEST_OPENGL_ERROR();
    trunk_program.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, trunk_program.texture_id());
    TEST_OPENGL_ERROR();
    glActiveTexture(GL_TEXTURE1);
    TEST_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_2D, trunk_program.lighting_id());
    TEST_OPENGL_ERROR();
    trunk_program.mat4vf("model_view_matrix", mv);
    trunk_program.mat4vf("projection_matrix", g_proj);
    trunk_program.init_3f("sun_dir", g_sun_dir);
    trunk_program.init_3f("sun_color", g_sun_color);
    TEST_OPENGL_ERROR();
    glBindVertexArray(trunk_program.vao_id());
    TEST_OPENGL_ERROR();
    glDrawArrays(GL_TRIANGLES, 0, trunk_v.size() / 3);
    TEST_OPENGL_ERROR();

    glDisable(GL_CULL_FACE);
    TEST_OPENGL_ERROR();
    leaves_program.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, leaves_program.texture_id());
    TEST_OPENGL_ERROR();
    glActiveTexture(GL_TEXTURE1);
    TEST_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_2D, leaves_program.lighting_id());
    TEST_OPENGL_ERROR();
    leaves_program.mat4vf("model_view_matrix", mv);
    leaves_program.mat4vf("projection_matrix", g_proj);
    leaves_program.init_3f("sun_dir", g_sun_dir);
    leaves_program.init_3f("sun_color", g_sun_color);
    glBindVertexArray(leaves_program.vao_id());
    TEST_OPENGL_ERROR();
    glDrawArrays(GL_TRIANGLES, 0, leaf_v.size() / 3);
    TEST_OPENGL_ERROR();

//    mygl::matrix4 mv = g_view * mygl::translate(0, -2, 0);
//    glEnable(GL_CULL_FACE);
//    glCullFace(GL_FRONT);
//    q_program.use();
//    q_program.mat4vf("model_view_matrix", mv);
//    q_program.mat4vf("projection_matrix", g_proj);
//
//    GLint thick = glGetUniformLocation(q_program.prog_id(), "outline_width");
//    if (thick != -1)
//        glUniform1f(thick, 0.05f);
//    glBindVertexArray(q_program.vao_id());
//    //glDrawArrays(GL_TRIANGLES, 0, vertices.size());
//
//    glDisable(GL_CULL_FACE);
//    g_program.use();
//    g_program.mat4vf("model_view_matrix", mv);
//    g_program.mat4vf("projection_matrix", g_proj);
//    g_program.init_3f("sun_dir", g_sun_dir);
//    g_program.init_3f("sun_color", g_sun_color);
//
//    glBindVertexArray(g_program.vao_id());
//    glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 3 );

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        TEST_OPENGL_ERROR();
        glViewport(0, 0, 1024, 1024);
        TEST_OPENGL_ERROR();
        glClear(GL_COLOR_BUFFER_BIT);
        TEST_OPENGL_ERROR();
        glDisable(GL_DEPTH_TEST);
        TEST_OPENGL_ERROR();
    	glDisable(GL_CULL_FACE);
        TEST_OPENGL_ERROR();
        post_program.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, scene_color_tex);
        TEST_OPENGL_ERROR();
        GLint loc = glGetUniformLocation(post_program.prog_id(), "scene_tex");
        glUniform1i(loc, 0);

     glBindVertexArray(sky_vao);
     TEST_OPENGL_ERROR();
     glDrawArrays(GL_TRIANGLES, 0, 3);
     TEST_OPENGL_ERROR();

    glEnable(GL_DEPTH_TEST);
    TEST_OPENGL_ERROR();
    glEnable(GL_CULL_FACE);
    TEST_OPENGL_ERROR();
    glutSwapBuffers();
    TEST_OPENGL_ERROR();
}

int main(int argc, char* argv[])
{
    if (!init_glut(argc, argv))
        return 1;
    if (!init_glew())
        return 1;
    if (!init_gl())
        return 1;
    init_fbo();
    g_sun_dir.normalize();

    const std::filesystem::path base =
        std::filesystem::absolute(argv[0]).parent_path();
    auto asset = [&](const char* rel) { return (base / rel).string(); };

    try
    {
        trunk_program = init_shaders(asset("shaders/vertex.shd"),
                                 asset("shaders/fragment.shd"));
        leaves_program = init_shaders(asset("shaders/vertex.shd"),
                                 asset("shaders/fragment.shd"));
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
    glGenVertexArrays(1, &sky_vao);
    if (!load_obj("Pine_4.obj", vertices, normals_flat, uv))
    {
        std::cerr << "Could not load map.obj" << std::endl;
        return 1;
    }
    if (!load_obj(asset("real_pine_bark.obj").c_str(), trunk_v, trunk_n, trunk_uv))
    {
        std::cerr << "Could not load laf.obj" << std::endl;
        return 1;
    }
    if (!load_obj(asset("real_pine_leaves.obj").c_str(), leaf_v, leaf_n, leaf_uv))
    {
        std::cerr << "Could not load trunk.obj" << std::endl;
        return 1;
    }
    auto L = 2 * M_PI * 4.0f * 1.0f;
    auto longueur = L / 4.0f;
    std::cerr << vertices.size();
    g_program.init_object(vertices, normals_flat, uv);
    q_program.init_object(vertices, normals_flat, uv);
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

    tifo::rgb24_image* bark    = tifo::load_image("bark.tga");
    tifo::rgb24_image* leaf    = tifo::load_image("leaf.tga");

    trunk_program.init_single_texture(bark, lighting);
    leaves_program.init_single_texture(leaf, lighting);


    ground(g_verts, g_normals, g_uv, 200.0f);
    ground_program.init_object(g_verts, g_normals, g_uv);
    TEST_OPENGL_ERROR();
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