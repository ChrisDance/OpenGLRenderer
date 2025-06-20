#pragma once

#include <glm/glm.hpp>
#include "mygl.h"

namespace Input
{
    struct input_config
    {
        int CursorMode{GLFW_CURSOR_NORMAL};
        bool EnableMouseCallbacks{true};
        bool EnableScrollCallbacks{true};
        double MouseSensitivity{1.0};
        double ScrollSensitivity{1.0};
    };

    void key_callback(GLFWwindow *win, int key, int scancode, int action, int mode);
    void mouse_button_callback(GLFWwindow *win, int button, int action, int mods);
    void mouse_callback(GLFWwindow *win, double xpos, double ypos);
    void scroll_callback(GLFWwindow *win, double xoffset, double yoffset);

    void init(GLFWwindow *win, input_config cfg = {});
    void update();

    void set_cursor_mode(int cursor_mode);
    void set_mouse_sensitivity(double sensitivity);
    void set_scroll_sensitivity(double sensitivity);
    void set_config(const input_config &new_config);

    bool is_key_pressed(int key);
    bool is_key_just_pressed(int key);
    bool is_mouse_button_pressed(int button);
    bool is_mouse_button_just_pressed(int button);

    glm::vec2 get_mouse_position();
    glm::vec2 get_mouse_delta();
    glm::vec2 get_scroll_offset();
    void reset_mouse();

    input_config get_config();
}
