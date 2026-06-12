#pragma once

class Camera;
struct RenderParams;

// Dear ImGui panel exposing all rendering parameters. Call gui_init() once
// after the OpenGL context exists, then gui_render() every frame between the
// scene render and the buffer swap.
void gui_init(int win_w, int win_h);
void gui_render(RenderParams& params, Camera& camera, bool visible);

// True when ImGui wants exclusive use of the corresponding input device.
bool gui_wants_mouse();
bool gui_wants_keyboard();

// GLUT event forwarding.
void gui_reshape(int w, int h);
void gui_on_mouse(int button, int state, int x, int y);
void gui_on_motion(int x, int y);
void gui_on_keyboard(unsigned char c, int x, int y);
void gui_on_keyboard_up(unsigned char c, int x, int y);
void gui_on_special(int key, int x, int y);
void gui_on_special_up(int key, int x, int y);
