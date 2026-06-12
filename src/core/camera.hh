#pragma once

#include "matrix4.hh"

class Camera
{
public:
    Camera(int win_w = 1024, int win_h = 1024);

    void on_key_down(unsigned char key);
    void on_key_up(unsigned char key);
    void on_passive_motion(int x, int y);

    void update();

    void set_viewport(int w, int h);

    // Disable mouse look while the GUI owns the cursor.
    void set_look_enabled(bool enabled);
    float& speed()
    {
        return speed_;
    }
    float& mouse_speed()
    {
        return mouse_speed_;
    }

    const mygl::matrix4& view() const
    {
        return view_;
    }
    const mygl::matrix4& proj() const
    {
        return proj_;
    }
    mygl::vector3 position() const
    {
        return mygl::vector3(pos_x_, pos_y_, pos_z_);
    }

private:
    float pos_x_ = 0.0f;
    float pos_y_ = 1.0f;
    float pos_z_ = 5.0f;
    float horizontal_angle_ = 3.14f;
    float vertical_angle_ = 0.0f;
    float speed_ = 3.0f;
    float mouse_speed_ = 0.003f;
    int win_w_;
    int win_h_;
    float aspect_ = 1.0f;

    int last_mouse_x_ = -1;
    int last_mouse_y_ = -1;
    int mouse_x_;
    int mouse_y_;

    bool key_z_ = false;
    bool key_s_ = false;
    bool key_q_ = false;
    bool key_d_ = false;
    bool look_enabled_ = true;

    mygl::matrix4 view_;
    mygl::matrix4 proj_;
};
