#include "model.hpp"
#include "mygl.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "particle_emitter.hpp"
#include "lib/stb_image.h"
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
#include "particle_emitter.hpp"
#include "hash_grid.hpp"
#include <entt/entt.hpp>
#include "game.hpp"
// #define MAX_OBJECTS_IN_SCENE 10


// unsigned int particleShader;
// std::vector<std::unique_ptr<ParticleEmitter>> particleEmitters;


// Add this function before main()
// void setupParticleSystem() {
//     // Create particle shader
//     particleShader = Shader::create("particle_vertex.glsl", "particle_fragment.glsl");

//     // Create some particle emitters with different effects

//     // Fire emitter at origin
//     auto fireEmitter = std::make_unique<ParticleEmitter>(ParticlePresets::createFire());
//     fireEmitter->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
//     particleEmitters.push_back(std::move(fireEmitter));

//     // Smoke emitter slightly above fire
//     auto smokeEmitter = std::make_unique<ParticleEmitter>(ParticlePresets::createExplosion());
//     smokeEmitter->setPosition(glm::vec3(0.0f, 1.0f, 0.0f));
//     particleEmitters.push_back(std::move(smokeEmitter));

//     // // Snow falling from above
//     auto snowEmitter = std::make_unique<ParticleEmitter>(ParticlePresets::createSnow());
//     particleEmitters.push_back(std::move(snowEmitter));
// }


// Add this function to handle particle input (optional)
// void handleParticleInput(GLFWwindow *window) {
//     static bool spacePressed = false;

//     // Trigger explosion on spacebar
//     if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spacePressed) {
//         auto explosionEmitter = std::make_unique<ParticleEmitter>(ParticlePresets::createExplosion());
//         explosionEmitter->setPosition(camera.Position + camera.Front * 2.0f);
//         particleEmitters.push_back(std::move(explosionEmitter));
//         spacePressed = true;
//     }
//     if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
//         spacePressed = false;
//     }

//     // Trigger sparks on 'F' key
//     static bool fPressed = false;
//     if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !fPressed) {
//         auto sparksEmitter = std::make_unique<ParticleEmitter>(ParticlePresets::createSparks());
//         sparksEmitter->setPosition(camera.Position + camera.Front * 3.0f);
//         particleEmitters.push_back(std::move(sparksEmitter));
//         fPressed = true;
//     }
//     if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE) {
//         fPressed = false;
//     }
// }

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



// void add_instance(Model &m, glm::vec3 trans, float scale, glm::vec3 rot = glm::vec3(0))
// {
//     std::vector<glm::mat4> instances;
//     glm::mat4 model = glm::mat4(1.0f);

//     model = glm::translate(model, trans);
//     model = glm::rotate(model, (float)M_PI, rot);
//     model = glm::scale(model, glm::vec3(scale));

//     //
//     m.aabb = m.aabb.transform(model);

//     instances.push_back(model);

//     uploadInstanceData(&m, instances);
// }
// void setupLighting(unsigned int shaderID) {
//     // Directional light
//     Shader::setVec3("directLight.Direction", shaderID, glm::vec3(-1.0f, 0.0f, -0.0f));
//     Shader::setVec3("directLight.Intensity", shaderID, glm::vec3(0.5f, 0.5f, 0.5f));
//     Shader::setVec3("directLight.Color", shaderID, glm::vec3(1.0f, 1.0f, 1.0f));

//     // Number of point lights
//     Shader::setInt("numPointLights", shaderID, 2);

//     // Point light 0
//     Shader::setVec3("pointLights[0].position", shaderID, glm::vec3(2.0f, 1.0f, 3.0f));
//     Shader::setVec3("pointLights[0].color", shaderID, glm::vec3(1.0f, 0.5f, 0.3f));
//     Shader::setFloat("pointLights[0].constant", shaderID, 1.0f);
//     Shader::setFloat("pointLights[0].linear", shaderID, 0.09f);
//     Shader::setFloat("pointLights[0].quadratic", shaderID, 0.032f);

//     // Point light 1
//     Shader::setVec3("pointLights[1].position", shaderID, glm::vec3(-2.0f, 2.0f, -1.0f));
//     Shader::setVec3("pointLights[1].color", shaderID, glm::vec3(0.3f, 0.5f, 1.0f));
//     Shader::setFloat("pointLights[1].constant", shaderID, 1.0f);
//     Shader::setFloat("pointLights[1].linear", shaderID, 0.09f);
//     Shader::setFloat("pointLights[1].quadratic", shaderID, 0.032f);
// }
// inline float distanceToPlane(const glm::vec4& plane, const glm::vec3& point) {
//     return glm::dot(glm::vec3(plane), point) + plane.w;
// }

// inline bool isAABBOutsidePlane(const AABB& aabb, const glm::vec4& plane) {
//     // Get the vertex closest to the plane (negative vertex)
//     glm::vec3 negativeVertex;
//     negativeVertex.x = (plane.x < 0.0f) ? aabb.max.x : aabb.min.x;
//     negativeVertex.y = (plane.y < 0.0f) ? aabb.max.y : aabb.min.y;
//     negativeVertex.z = (plane.z < 0.0f) ? aabb.max.z : aabb.min.z;

//     // If the closest vertex is outside (negative distance), the entire AABB is outside
//     return distanceToPlane(plane, negativeVertex) < 0.0f;
// }

// // Test if AABB is within or intersects the frustum
// bool isAABBInFrustum(const AABB& aabb, const Frustum& frustum) {
//     // Test against all 6 frustum planes
//     for (int i = 0; i < 6; i++) {
//         if (isAABBOutsidePlane(aabb, frustum.planes[i])) {
//             return false; // AABB is completely outside this plane
//         }
//     }
//     return true; // AABB is inside or intersects the frustum
// }

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
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // Standard alpha blending


    // auto ID = Shader::create("vertex.glsl", "fragment.glsl");

    // Model ourModel = load_model("ford/scene.gltf");

    // SpatialHashGrid spatialGrid(15.0f); // 15-unit cells

    // // Add objects (you'd do this as you load models)


    Camera camera(glm::vec3(0.0f, 0.0f, 10.0f));
    entt::locator<Camera>::emplace(camera);

    // setupModel(&ourModel, 10);
    // add_instance(ourModel, glm::vec3(0), 0.01f, glm::vec3(0, 0, 1));

    // Shader::use(ID);
    // setupLighting(ID);
    // setupParticleSystem();
    // // ParticleEmitter emitter = ParticleEmitter(ParticlePresets::createFire());
    // spatialGrid.addObject(1, ourModel.aabb, &ourModel);
    game_init(window);
    fps_counter_init();
    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        // handleParticleInput(window);  // Add this line

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        game_update(deltaTime /* seconds */);

        // Shader::use(ID);
        // setupLighting(ID);

        // // View/projection transformations
        // glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        // glm::mat4 view = camera.GetViewMatrix();

        // Shader::setMat4("projection", ID, projection);
        // Shader::setMat4("view", ID, view);
        // Shader::setVec3("viewPos", ID, camera.Position);



        // camera.CalculateFrustum(projection, view);

        // auto visibleObjects = spatialGrid.cullObjects(camera);

        // // Render only visible objects
        // for (ObjectHandle handle : visibleObjects) {
        //     const SpatialObject* obj = spatialGrid.getObject(handle);
        //     Model* model = static_cast<Model*>(obj->userData);
        //     drawModel(ID, model, 1);
        // }

       // if(isAABBInFrustum(test, camera.ViewFrustum))
       // {
       //     std::cout << "Test is in view frustum" << std::endl;
       // } else {
       //     std::cout << "Test is not in view frustum" << std::endl;
       // }


        // if(isAABBInFrustum(ourModel.aabb, camera.ViewFrustum))
        // {
            // drawModel(ID, &ourModel, 1);

            // std::cout << "YES " << std::endl;
        // } else {
        //     std::cout << "NO " << std::endl;
        // }

        // for (auto it = particleEmitters.begin(); it != particleEmitters.end();) {
        //             (*it)->update(deltaTime);

        //             // Remove dead emitters (non-looping ones that have finished)
        //             if (!(*it)->isAlive()) {
        //                 it = particleEmitters.erase(it);
        //             } else {
        //                 (*it)->render(particleShader, view, projection);
        //                 ++it;
        //             }
        //         }






        glfwSwapBuffers(window);
        fps_counter_update();
        glfwPollEvents();
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
