#include <cmath>
#include <iostream>

#include "img/image.hh"
#include "img/image_io.hh"
#include "init.hh"
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


void ground(std::vector<GLfloat>& vertices,
            std::vector<GLfloat>& normals,
            std::vector<GLfloat>& uv,
            float size)
{
    vertices.clear();
    normals.clear();
    uv.clear();
    float h = size / 2.0f;

    float verts[18] = {
        -h, 0, -h,   h, 0, -h,   h, 0,  h,
        -h, 0, -h,   h, 0,  h,  -h, 0,  h
    };
    for (int i = 0; i < 18; i++) vertices.push_back(verts[i]);
    for (int i = 0; i < 6; i++) {
        normals.push_back(0); normals.push_back(1); normals.push_back(0);
    }
    for (int i = 0; i < 6; i++) {
        uv.push_back(0); uv.push_back(0);
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
              std::vector<GLfloat>& uv_buffer_data,
              GLfloat radius,
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
        float u0 = (float)i/segments;
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
    if (key == 'z' || key == 'Z' || key == 'w' || key == 'W') key_z = true;
    if (key == 's' || key == 'S') key_s = true;
    if (key == 'q' || key == 'Q' || key == 'a' || key == 'A') key_q = true;
    if (key == 'd' || key == 'D') key_d = true;
    if (key == 27) exit(0);
}

void keyboard_up(unsigned char key, int, int)
{
    if (key == 'z' || key == 'Z' || key == 'w' || key == 'W') key_z = false;
    if (key == 's' || key == 'S') key_s = false;
    if (key == 'q' || key == 'Q' || key == 'a' || key == 'A') key_q = false;
    if (key == 'd' || key == 'D') key_d = false;
}
void special_down(int key, int, int)
{
    if (key == GLUT_KEY_UP)    key_up    = true;
    if (key == GLUT_KEY_DOWN)  key_down  = true;
    if (key == GLUT_KEY_LEFT)  key_left  = true;
    if (key == GLUT_KEY_RIGHT) key_right = true;
}

void special_up(int key, int, int)
{
    if (key == GLUT_KEY_UP)    key_up    = false;
    if (key == GLUT_KEY_DOWN)  key_down  = false;
    if (key == GLUT_KEY_LEFT)  key_left  = false;
    if (key == GLUT_KEY_RIGHT) key_right = false;
}

void keyboard(unsigned char key, int, int)
{
    if (key == 27) exit(0);
}

int mouse_x = win_w / 2;
int mouse_y = win_h / 2;

void passive_motion(int x, int y)
{
    mouse_x = x;
    mouse_y = y;
}void compute_matrices_from_inputs()
{
    static int last_time = glutGet(GLUT_ELAPSED_TIME);
    int now = glutGet(GLUT_ELAPSED_TIME);
    float delta_time = (now - last_time) / 1000.0f;
    last_time = now;

    if (last_mouse_x >= 0) {
        int dx = mouse_x - last_mouse_x;
        int dy = mouse_y - last_mouse_y;
        if (std::abs(dx) < 200 && std::abs(dy) < 200) {
            horizontal_angle -= dx * mouse_speed;
            vertical_angle   -= dy * mouse_speed;
        }
    }
    last_mouse_x = mouse_x;
    last_mouse_y = mouse_y;

    const int margin = 80;
    const float edge_speed = 1.5f;
    if (mouse_x < margin)
        horizontal_angle += edge_speed * delta_time * (margin - mouse_x) / float(margin);
    if (mouse_x > win_w - margin)
        horizontal_angle -= edge_speed * delta_time * (mouse_x - (win_w - margin)) / float(margin);
    if (mouse_y < margin)
        vertical_angle += edge_speed * delta_time * (margin - mouse_y) / float(margin);
    if (mouse_y > win_h - margin)
        vertical_angle -= edge_speed * delta_time * (mouse_y - (win_h - margin)) / float(margin);

    if (vertical_angle >  1.5f) vertical_angle =  1.5f;
    if (vertical_angle < -1.5f) vertical_angle = -1.5f;

    float fx = std::cos(vertical_angle) * std::sin(horizontal_angle);
    float fy = std::sin(vertical_angle);
    float fz = std::cos(vertical_angle) * std::cos(horizontal_angle);

    float rx = std::sin(horizontal_angle - 1.5708f);
    float rz = std::cos(horizontal_angle - 1.5708f);

    if (key_z) { pos_x += fx * delta_time * speed;
                 pos_y += fy * delta_time * speed;
                 pos_z += fz * delta_time * speed; }
    if (key_s) { pos_x -= fx * delta_time * speed;
                 pos_y -= fy * delta_time * speed;
                 pos_z -= fz * delta_time * speed; }
    if (key_d) { pos_x += rx * delta_time * speed;
                 pos_z += rz * delta_time * speed; }
    if (key_q) { pos_x -= rx * delta_time * speed;
                 pos_z -= rz * delta_time * speed; }

    g_view = mygl::look_at(
        pos_x, pos_y, pos_z,
        pos_x + fx, pos_y + fy, pos_z + fz,
        0, 1, 0);
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
    glBindFramebuffer(GL_FRAMEBUFFER, scene_fbo);
    glGenTextures(1, &scene_color_tex);
    glBindTexture(GL_TEXTURE_2D, scene_color_tex);
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 1024, 1024, 0, GL_RGB, GL_FLOAT, NULL);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, scene_color_tex, 0);
    glGenRenderbuffers(1, &scene_depth_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, scene_depth_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1024, 1024);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER, scene_depth_rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "FBO incomplete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, scene_fbo);
    glViewport(0, 0, 1024, 1024);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    sky_program.use();
    mygl::matrix4 inv_vp = (g_proj * g_view).inverse();
	sky_program.mat4vf("inv_view_proj", inv_vp);
	sky_program.init_3f("sun_dir", g_sun_dir);
	sky_program.init_3f("cam_pos", mygl::vector3(pos_x, pos_y, pos_z));
    glBindVertexArray(sky_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);

    glCullFace(GL_BACK);
    mygl::matrix4 mv_ground = g_view * mygl::translate(0, -2, 0);
    glDisable(GL_CULL_FACE);
    ground_program.use();
    ground_program.mat4vf("model_view_matrix", mv_ground);
    ground_program.mat4vf("projection_matrix", g_proj);
    ground_program.init_3f("sun_dir", g_sun_dir);
	ground_program.init_3f("sun_color", g_sun_color);
    glBindVertexArray(ground_program.vao_id());
    glDrawArrays(GL_TRIANGLES, 0, g_verts.size() / 3);

    mygl::matrix4 mv = g_view * mygl::translate(0, 0, 0);
	glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    q_program.use();
    q_program.mat4vf("model_view_matrix", mv);
    q_program.mat4vf("projection_matrix", g_proj);

    GLint thick = glGetUniformLocation(q_program.prog_id(), "outline_width");
    if (thick != -1) glUniform1f(thick, 0.05f);
    glBindVertexArray(q_program.vao_id());
    glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 3);

    glCullFace(GL_FRONT);
    g_program.use();
    g_program.mat4vf("model_view_matrix", mv);
    g_program.mat4vf("projection_matrix", g_proj);
    g_program.init_3f("sun_dir", g_sun_dir);
	g_program.init_3f("sun_color", g_sun_color);

    glBindVertexArray(g_program.vao_id());
    glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 3);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, 1024, 1024);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
    post_program.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, scene_color_tex);
    GLint loc = glGetUniformLocation(post_program.prog_id(), "scene_tex");
    glUniform1i(loc, 0);

    glBindVertexArray(sky_vao);  // ou un autre VAO vide
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glutSwapBuffers();
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
    try
    {
        g_program = init_shaders(
            "/home/ad/image/OPENGL/TP/pogl_skel_tp/tp2/vertex.shd",
            "/home/ad/image/OPENGL/TP/pogl_skel_tp/tp2/fragment.shd");
        q_program = init_shaders(
            "/home/ad/image/OPENGL/TP/pogl_skel_tp/tp2/outlineVertexShader.shd",
            "/home/ad/image/OPENGL/TP/pogl_skel_tp/tp2/"
            "outlineFragmentShader.shd");
        sky_program = init_shaders(
            "/home/ad/image/OPENGL/TP/pogl_skel_tp/tp2/sky_vertex.shd",
            "/home/ad/image/OPENGL/TP/pogl_skel_tp/tp2/sky_fragment.shd"
        );
        ground_program = init_shaders(
            "/home/ad/image/OPENGL/TP/pogl_skel_tp/tp2/groundVertex.shd",
            "/home/ad/image/OPENGL/TP/pogl_skel_tp/tp2/groundFragment.shd"
        );
        post_program = init_shaders(
            "/home/ad/image/OPENGL/TP/pogl_skel_tp/tp2/post_vertex.shd",
            "/home/ad/image/OPENGL/TP/pogl_skel_tp/tp2/post_fragment.shd"

        );
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
    cylinder(vertices, normals_flat, uv, 1.0f, 4.0f, 360);
    auto L = 2 * M_PI * 4.0f * 1.0f;
    auto longueur = L / 4.0f;
    std::cerr << vertices.size();
    g_program.init_object(vertices, normals_flat, uv);
    q_program.init_object(vertices, normals_flat, uv);
    g_program.init_POV(mygl::vector3(4.0f, 1.0f, 4.5f),
                       mygl::vector3(0.0f, 0.0f, 0.0f),
                       mygl::vector3(0.0f, 1.0f, 0.0f), -1.0f, 1.0f, -1.0f,
                       1.0f, 1.0f, 250.0f);
    tifo::rgb24_image* texture = tifo::load_image("texture.tga");
    tifo::rgb24_image* lighting = tifo::load_image("lighting.tga");
    g_program.init_texture(texture, lighting);

	ground(g_verts, g_normals, g_uv, 200.0f);
	ground_program.init_object(g_verts, g_normals, g_uv);

 glutDisplayFunc(display);
glutIdleFunc(idle);
glutKeyboardFunc(keyboard_down);
glutKeyboardUpFunc(keyboard_up);
glutPassiveMotionFunc(passive_motion);
glutIgnoreKeyRepeat(1);

    glutMainLoop();

    return 0;
}