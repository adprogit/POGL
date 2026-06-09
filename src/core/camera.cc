#include "camera.hh"

#include <cmath>
#include <cstdlib>

#ifdef __APPLE__
#    include <GLUT/glut.h>
#else
#    include <GL/freeglut.h>
#endif

Camera::Camera(int win_w, int win_h)
    : win_w_(win_w)
    , win_h_(win_h)
    , mouse_x_(win_w / 2)
    , mouse_y_(win_h / 2)
{}

void Camera::set_viewport(int w, int h)
{
    win_w_ = w;
    win_h_ = h;
    if (h > 0)
    {
        aspect_ = (float)w / (float)h;
    }
    else
    {
        aspect_ = 1.0f;
    }
}

void Camera::on_key_down(unsigned char key)
{
    if (key == 'z' || key == 'Z' || key == 'w' || key == 'W')
    {
        key_z_ = true;
    }
    if (key == 's' || key == 'S')
    {
        key_s_ = true;
    }
    if (key == 'q' || key == 'Q' || key == 'a' || key == 'A')
    {
        key_q_ = true;
    }
    if (key == 'd' || key == 'D')
    {
        key_d_ = true;
    }
    if (key == 27)
    {
        exit(0);
    }
}

void Camera::on_key_up(unsigned char key)
{
    if (key == 'z' || key == 'Z' || key == 'w' || key == 'W')
    {
        key_z_ = false;
    }
    if (key == 's' || key == 'S')
    {
        key_s_ = false;
    }
    if (key == 'q' || key == 'Q' || key == 'a' || key == 'A')
    {
        key_q_ = false;
    }
    if (key == 'd' || key == 'D')
    {
        key_d_ = false;
    }
}

void Camera::on_passive_motion(int x, int y)
{
    mouse_x_ = x;
    mouse_y_ = y;
}

void Camera::update()
{
    static int last_time = glutGet(GLUT_ELAPSED_TIME);
    int now = glutGet(GLUT_ELAPSED_TIME);
    float delta_time = (now - last_time) / 1000.0f;
    last_time = now;

    if (last_mouse_x_ >= 0)
    {
        int dx = mouse_x_ - last_mouse_x_;
        int dy = mouse_y_ - last_mouse_y_;
        if (std::abs(dx) < 200 && std::abs(dy) < 200)
        {
            horizontal_angle_ -= dx * mouse_speed_;
            vertical_angle_ -= dy * mouse_speed_;
        }
    }
    last_mouse_x_ = mouse_x_;
    last_mouse_y_ = mouse_y_;

    const int margin = 80;
    const float edge_speed = 1.5f;
    if (mouse_x_ < margin)
    {
        horizontal_angle_ +=
            edge_speed * delta_time * (margin - mouse_x_) / float(margin);
    }
    if (mouse_x_ > win_w_ - margin)
    {
        horizontal_angle_ -= edge_speed * delta_time
            * (mouse_x_ - (win_w_ - margin)) / float(margin);
    }
    if (mouse_y_ < margin)
    {
        vertical_angle_ +=
            edge_speed * delta_time * (margin - mouse_y_) / float(margin);
    }
    if (mouse_y_ > win_h_ - margin)
    {
        vertical_angle_ -= edge_speed * delta_time
            * (mouse_y_ - (win_h_ - margin)) / float(margin);
    }

    if (vertical_angle_ > 1.5f)
    {
        vertical_angle_ = 1.5f;
    }
    if (vertical_angle_ < -1.5f)
    {
        vertical_angle_ = -1.5f;
    }

    float fx = std::cos(vertical_angle_) * std::sin(horizontal_angle_);
    float fy = std::sin(vertical_angle_);
    float fz = std::cos(vertical_angle_) * std::cos(horizontal_angle_);

    float rx = std::sin(horizontal_angle_ - 1.5708f);
    float rz = std::cos(horizontal_angle_ - 1.5708f);

    if (key_z_)
    {
        pos_x_ += fx * delta_time * speed_;
        pos_y_ += fy * delta_time * speed_;
        pos_z_ += fz * delta_time * speed_;
    }
    if (key_s_)
    {
        pos_x_ -= fx * delta_time * speed_;
        pos_y_ -= fy * delta_time * speed_;
        pos_z_ -= fz * delta_time * speed_;
    }
    if (key_d_)
    {
        pos_x_ += rx * delta_time * speed_;
        pos_z_ += rz * delta_time * speed_;
    }
    if (key_q_)
    {
        pos_x_ -= rx * delta_time * speed_;
        pos_z_ -= rz * delta_time * speed_;
    }

    view_ = mygl::look_at(pos_x_, pos_y_, pos_z_, pos_x_ + fx, pos_y_ + fy,
                          pos_z_ + fz, 0, 1, 0);
    proj_ = mygl::frustum(-aspect_, aspect_, -1, 1, 1.0f, 250.0f);
}
