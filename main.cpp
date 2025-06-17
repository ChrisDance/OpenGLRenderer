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
std::vector<glm::mat4> instances;

int i = 1;
void add_instance()
{

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, (float)M_PI, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::translate(model, glm::vec3(0.0f, i * 3.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.01));

    instances.push_back(model);
    i++;
    uploadInstanceData(&ourModel, instances);
}
PointLight createPointLight(glm::vec3 position, glm::vec3 color, float intensity = 1.0f)
{
    PointLight light;
    light.position = position;
    light.color = color * intensity;

    // Calculate range based on intensity for better attenuation
    float range = intensity * 10.0f; // Adjust multiplier as needed

    // Use more reasonable attenuation values
    light.constant = 1.0f;
    light.linear = 4.5f / range;
    light.quadratic = 75.0f / (range * range);

    return light;
}
// Corrected uploadLightData function
void uploadLightData(std::vector<PointLight> &lights, unsigned int shaderID)
{
    // Upload number of lights
    glUniform1i(glGetUniformLocation(shaderID, "numPointLights"), static_cast<int>(lights.size()));

    // Upload each light individually
    for (size_t i = 0; i < lights.size() && i < 32; ++i)
    {
        std::string base = "pointLights[" + std::to_string(i) + "]";

        glUniform3f(glGetUniformLocation(shaderID, (base + ".position").c_str()),
                    lights[i].position.x, lights[i].position.y, lights[i].position.z);

        glUniform3f(glGetUniformLocation(shaderID, (base + ".color").c_str()),
                    lights[i].color.x, lights[i].color.y, lights[i].color.z);

        glUniform1f(glGetUniformLocation(shaderID, (base + ".constant").c_str()),
                    lights[i].constant);

        glUniform1f(glGetUniformLocation(shaderID, (base + ".linear").c_str()),
                    lights[i].linear);

        glUniform1f(glGetUniformLocation(shaderID, (base + ".quadratic").c_str()),
                    lights[i].quadratic);
    }
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

    // ourModel.maxInstances = 2;
    setupModel(&ourModel, 10);
    add_instance();
    std::vector<PointLight> lights;
    lights.push_back(createPointLight(glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(1.0f), 2.0f));
    // lights.push_back(createPointLight(glm::vec3(10.0f, 10.0f, 0.0f), glm::vec3(1.0f), 5.0f));

    Shader::use(ID);
    uploadLightData(lights, ID);
    // Shader::setVec3("lightPos", ID, glm::vec3(1.2f, 100.0f, 2.0f));
    // Shader::setVec3("viewPos", ID, camera.Position);
    // Shader::setVec3("lightColor", ID, glm::vec3(1.0f, 1.0f, 1.0f));
    // Shader::setVec3("objectColor", ID, glm::vec3(0.0f, 0.0f, 0.0f));

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
        Shader::setVec3("viewPos", ID, camera.Position);

        // World transformation
        // Shader::setMat4("model", ID, model);

        // Lighting
        // Shader::setVec3("lightPos", ID, glm::vec3(1.2f, 100.0f, 2.0f));

        // Shader::setVec3("lightColor", ID, glm::vec3(1.0f, 1.0f, 1.0f));
        // Shader::setVec3("objectColor", ID, glm::vec3(0.0f, 0.0f, 0.0f));

        drawModel(ID, &ourModel, instances);

        glfwSwapBuffers(window);
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