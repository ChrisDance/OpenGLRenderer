#include "model.hpp"
#include "mygl.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "shader.hpp"
#include "camera.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <memory>
#include "model_loader.hpp"

#define MAX_OBJECTS_IN_SCENE 10

// fps_counter.h
#pragma once

void fps_counter_init();
void fps_counter_update();

// fps_counter.cpp


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

struct Scene
{
    std::vector<PointLight> pointLights;
    std::vector<Model> models;
};

// Global variables
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 800;

Camera camera(glm::vec3(0.0f, 0.0f, 10.0f));
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

Model ourModel;

void add_instance(Model &m, glm::vec3 trans, float scale, glm::vec3 rot = glm::vec3(0))
{
    std::vector<glm::mat4> instances;
    glm::mat4 model = glm::mat4(1.0f);

    model = glm::rotate(model, (float)M_PI, rot);
    model = glm::translate(model, trans);
    model = glm::scale(model, glm::vec3(scale));

    instances.push_back(model);

    uploadInstanceData(&m, instances);
}



void setupLighting(unsigned int shaderID) {
    // Setup lighting data
    LightingData lighting = {};

    // Directional light
    lighting.directLight.Direction = glm::vec3(-0.2f, -1.0f, -0.3f);
    lighting.directLight.Intensity = glm::vec3(1.0f);
    lighting.directLight.Color = glm::vec3(0.0f, 0.0f, 1.0f);

    // Point lights
    lighting.numPointLights = 2;

    lighting.pointLights[0].position = glm::vec3(2.0f, 1.0f, 3.0f);
    lighting.pointLights[0].color = glm::vec3(1.0f, 0.5f, 0.3f);
    lighting.pointLights[0].constant = 1.0f;
    lighting.pointLights[0].linear = 0.09f;
    lighting.pointLights[0].quadratic = 0.032f;

    lighting.pointLights[1].position = glm::vec3(-2.0f, 2.0f, -1.0f);
    lighting.pointLights[1].color = glm::vec3(0.3f, 0.5f, 1.0f);
    lighting.pointLights[1].constant = 1.0f;
    lighting.pointLights[1].linear = 0.09f;
    lighting.pointLights[1].quadratic = 0.032f;

    lighting.viewPos = glm::vec3(0.0f, 0.0f, 3.0f);

    // Upload entire lighting data in one call
    Shader::setUniformBuffer("LightingData", shaderID, &lighting, sizeof(lighting), 0);

}


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
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Configure global OpenGL state
    glEnable(GL_DEPTH_TEST);

    // Create shaders (you'll need to create these files)
    auto ID = Shader::create("vertex.glsl", "fragment.glsl");

    // Load models (replace with your model path)
    // Model ourModel("ford/scene.gltf", true);

    ourModel = load_model("ford/scene.gltf");
    // Model m2 = load_model("snow_road/scene.gltf");

    // ourModel.maxInstances = 2;
    setupModel(&ourModel, 10);
    // setupModel(&m2, 1);
    add_instance(ourModel, glm::vec3(0), 0.01f, glm::vec3(0, 0, 1));
    // add_instance(m2, glm::vec3(0, 0.2, 0), 0.25f, glm::vec3(1, 0, 1));

    Shader::use(ID);
    setupLighting(ID);

    // Shader::setVec3("lightPos", ID, glm::vec3(1.2f, 100.0f, 2.0f));
    // Shader::setVec3("viewPos", ID, camera.Position);
    // Shader::setVec3("lightColor", ID, glm::vec3(1.0f, 1.0f, 1.0f));
    // Shader::setVec3("objectColor", ID, glm::vec3(0.0f, 0.0f, 0.0f));
    fps_counter_init();
    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Shader::use(ID);

        // View/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        Shader::setMat4("projection", ID, projection);
        Shader::setMat4("view", ID, view);
        // Shader::setVec3("viewPos", ID, camera.Position);




        drawModel(ID, &ourModel, 1);
        // drawModel(ID, &m2, 1);

        glfwSwapBuffers(window);
        fps_counter_update();
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{

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
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
{
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
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
