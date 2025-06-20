#include "Input.hpp"
#include "model.hpp"
#include "mygl.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "lib/stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "camera.hpp"
#include <iostream>

#include <entt/entt.hpp>
#include "game.hpp"


void fps_counter_init();
void fps_counter_update();


#include <iostream>

static double last_log_time = 0.0;
static int frame_count = 0;
static const double LOG_INTERVAL = 3.0; // seconds

void fps_counter_init() {
    last_log_time = glfwGetTime();
    frame_count = 0;
}

void fps_counter_update() {
    frame_count++;

    double current_time = glfwGetTime();
    double elapsed = current_time - last_log_time;

    if (elapsed >= LOG_INTERVAL) {
        double fps = frame_count / elapsed;
        std::cout << "FPS: " << fps << std::endl;

        // Reset for next interval
        last_log_time = current_time;
        frame_count = 0;
    }
}



// Global variables
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 600;


float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Input processing functions
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

int main()
{
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create window
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "3D OpenGL Renderer", NULL, NULL);
    entt::locator<Meta>::emplace(glm::vec2(SCR_WIDTH, SCR_HEIGHT));
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // work out how to intergrte this
    Input::input_config config;
    config.EnableMouseCallbacks = true;
    Input::init(window, config);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // AABB test(glm::vec3(1000), glm::vec3(1001));


    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Configure global OpenGL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);  // Add this for particle transparency
    glEnable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // Standard alpha blending
    Camera camera(glm::vec3(0.0f, 0.0f, 10.0f));
    entt::locator<Camera>::emplace(camera);

    game_init(window);
    fps_counter_init();
    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();

        processInput(window);


        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        game_update(deltaTime /* seconds */);
        Input::update();

        glfwSwapBuffers(window);
        fps_counter_update();


    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{


    Camera& camera = entt::locator<Camera>::value();
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(0, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(1, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(2, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(3, deltaTime);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    entt::locator<Meta>::emplace(glm::vec2(width, height));
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
{

    Camera& camera = entt::locator<Camera>::value();
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{

    Camera& camera = entt::locator<Camera>::value();
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
