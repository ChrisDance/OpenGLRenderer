#include "Input.hpp"
#include <bitset>
#include <cassert>

namespace Input
{
    namespace
    {
        GLFWwindow *window = nullptr;
        input_config config;

        std::bitset<1024> keys;
        std::bitset<1024> keysPressed;
        std::bitset<8> mouseButtons;
        std::bitset<8> mouseButtonsPressed;

        glm::vec2 mousePos{0.0f};
        glm::vec2 mouseDelta{0.0f};
        glm::vec2 lastMousePos{0.0f};
        glm::vec2 scrollOffset{0.0f};
        bool firstMouse{true};
    }

    void key_callback(GLFWwindow *win, int key, int scancode, int action, int mode)
    {
        assert(key >= 0 && key < 1024);

        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(win, true);

        if (action == GLFW_PRESS)
        {
            keys[key] = true;
            keysPressed[key] = true;
        }
        else if (action == GLFW_RELEASE)
        {
            keys[key] = false;
        }
    }

    void mouse_button_callback(GLFWwindow *win, int button, int action, int mods)
    {
        assert(button >= 0 && button < 8);

        if (action == GLFW_PRESS)
        {
            mouseButtons[button] = true;
            mouseButtonsPressed[button] = true;
        }
        else if (action == GLFW_RELEASE)
        {
            mouseButtons[button] = false;
        }
    }

    void mouse_callback(GLFWwindow *win, double xpos, double ypos)
    {
        glm::vec2 currentPos(xpos, ypos);

        if (firstMouse)
        {
            lastMousePos = currentPos;
            firstMouse = false;
        }

        mouseDelta = (currentPos - lastMousePos) * static_cast<float>(config.MouseSensitivity);
        lastMousePos = currentPos;
        mousePos = currentPos;
    }

    void scroll_callback(GLFWwindow *win, double xoffset, double yoffset)
    {
        scrollOffset.x += static_cast<float>(xoffset * config.ScrollSensitivity);
        scrollOffset.y += static_cast<float>(yoffset * config.ScrollSensitivity);
    }

    void init(GLFWwindow *win, input_config cfg)
    {
        window = win;
        config = cfg;

        glfwSetInputMode(window, GLFW_CURSOR, config.CursorMode);

        glfwSetKeyCallback(window, key_callback);

        if (config.EnableMouseCallbacks)
        {
            glfwSetMouseButtonCallback(window, mouse_button_callback);
            glfwSetCursorPosCallback(window, mouse_callback);
        }

        if (config.EnableScrollCallbacks)
        {
            glfwSetScrollCallback(window, scroll_callback);
        }
    }

    void update()
    {
        keysPressed.reset();
        mouseButtonsPressed.reset();
        scrollOffset = glm::vec2(0.0f);
        mouseDelta = glm::vec2(0.0f);
    }

    void set_cursor_mode(int cursorMode)
    {
        assert(window != nullptr);
        config.CursorMode = cursorMode;
        glfwSetInputMode(window, GLFW_CURSOR, cursorMode);
    }

    void set_mouse_sensitivity(double sensitivity)
    {
        config.MouseSensitivity = sensitivity;
    }

    void set_scroll_sensitivity(double sensitivity)
    {
        config.ScrollSensitivity = sensitivity;
    }

    void set_config(const input_config &newConfig)
    {
        config = newConfig;
        assert(window != nullptr);
        glfwSetInputMode(window, GLFW_CURSOR, config.CursorMode);
    }

    bool is_key_pressed(int key)
    {
        assert(key >= 0 && key < 1024);
        return keys[key];
    }

    bool is_key_just_pressed(int key)
    {
        assert(key >= 0 && key < 1024);
        return keysPressed[key];
    }

    bool is_mouse_button_pressed(int button)
    {
        assert(button >= 0 && button < 8);
        return button >= 0 && button < 8 && mouseButtons[button];
    }

    bool is_mouse_button_just_pressed(int button)
    {
        assert(button >= 0 && button < 8);
        return mouseButtonsPressed[button];
    }

    glm::vec2 get_mouse_position()
    {
        return mousePos;
    }

    glm::vec2 get_mouse_delta()
    {
        return mouseDelta;
    }

    glm::vec2 get_scroll_offset()
    {
        return scrollOffset;
    }

    void reset_mouse()
    {
        firstMouse = true;
        mouseDelta = glm::vec2(0.0f);
    }

    input_config get_config()
    {
        return config;
    }
}
