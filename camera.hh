#pragma once

#include "matrix4.hh"

// Camera FPS pilotee au clavier (ZQSD/WASD) + souris (regard).
// Regroupe l'etat global qui vivait dans main.cpp.
class Camera
{
public:
    Camera(int win_w = 1024, int win_h = 1024);

    // Callbacks d'entree (a brancher sur les callbacks GLUT).
    void on_key_down(unsigned char key); // quitte sur ESC
    void on_key_up(unsigned char key);
    void on_passive_motion(int x, int y);

    // Recalcule view/proj a partir des entrees et du temps ecoule.
    void update();

    const mygl::matrix4& view() const { return view_; }
    const mygl::matrix4& proj() const { return proj_; }
    mygl::vector3 position() const { return mygl::vector3(pos_x_, pos_y_, pos_z_); }

private:
    float pos_x_ = 0.0f, pos_y_ = 1.0f, pos_z_ = 5.0f;
    float horizontal_angle_ = 3.14f;
    float vertical_angle_ = 0.0f;
    float speed_ = 3.0f;
    float mouse_speed_ = 0.003f;
    int win_w_, win_h_;

    int last_mouse_x_ = -1, last_mouse_y_ = -1;
    int mouse_x_, mouse_y_;

    bool key_z_ = false, key_s_ = false, key_q_ = false, key_d_ = false;

    mygl::matrix4 view_, proj_;
};
