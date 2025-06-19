#include "game.hpp"
#include "camera.hpp"
#include "collision_system.hpp"
#include "model.hpp"
#include "model_loader.hpp"
#include "model_setup.hpp"
#include "resource_ids.hpp"
#include "shader.hpp"
#include <entt/entt.hpp>
#include <iostream>
#include "light_system.hpp"
#include "ground_system.hpp"


class AABBRenderer {
private:
    GLuint VAO, VBO, EBO;
    GLuint shaderProgram;
    GLint mvpLocation;
    GLint colorLocation;

    // Cube vertices (unit cube from -0.5 to 0.5)
    static constexpr float vertices[24] = {
        -0.5f, -0.5f, -0.5f,  // 0: min corner
         0.5f, -0.5f, -0.5f,  // 1
        -0.5f,  0.5f, -0.5f,  // 2
         0.5f,  0.5f, -0.5f,  // 3
        -0.5f, -0.5f,  0.5f,  // 4
         0.5f, -0.5f,  0.5f,  // 5
        -0.5f,  0.5f,  0.5f,  // 6
         0.5f,  0.5f,  0.5f   // 7: max corner
    };

    // Wireframe indices (12 edges of a cube)
    static constexpr unsigned int indices[24] = {
        // Bottom face edges
        0, 1,  1, 3,  3, 2,  2, 0,
        // Top face edges
        4, 5,  5, 7,  7, 6,  6, 4,
        // Vertical edges
        0, 4,  1, 5,  2, 6,  3, 7
    };

    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        uniform mat4 mvp;
        void main() {
            gl_Position = mvp * vec4(aPos, 1.0);
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec3 color;
        void main() {
            FragColor = vec4(color, 1.0);
        }
    )";

    GLuint compileShader(GLenum type, const char* source) {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);

        // Check compilation
        int success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            // Handle error (print or throw)
        }
        return shader;
    }

public:
    AABBRenderer() {
        // Create and compile shaders
        GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
        GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

        // Create shader program
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);

        // Check linking
        int success;
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
            // Handle error
        }

        // Clean up shaders
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        // Get uniform locations
        mvpLocation = glGetUniformLocation(shaderProgram, "mvp");
        colorLocation = glGetUniformLocation(shaderProgram, "color");

        // Create VAO, VBO, EBO
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        // Upload vertex data
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Upload index data
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // Set vertex attributes
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }

    ~AABBRenderer() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        glDeleteProgram(shaderProgram);
    }

    // Draw AABB wireframe
    void drawAABB(const AABB& aabb, const glm::mat4& viewProjectionMatrix,
                  const glm::vec3& color = glm::vec3(1.0f, 1.0f, 0.0f)) {

        // Create transform matrix for this AABB
        glm::vec3 center = aabb.getCenter();
        glm::vec3 size = aabb.getSize();

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, center);
        model = glm::scale(model, size);

        glm::mat4 mvp = viewProjectionMatrix * model;

        // Use shader program
        glUseProgram(shaderProgram);
        glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, &mvp[0][0]);
        glUniform3fv(colorLocation, 1, &color[0]);

        // Draw wireframe
        glBindVertexArray(VAO);
        glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    // Draw multiple AABBs with different colors
    void drawAABBs(const std::vector<std::pair<AABB, glm::vec3>>& aabbs,
                   const glm::mat4& viewProjectionMatrix) {
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        for (const auto& [aabb, color] : aabbs) {
            glm::vec3 center = aabb.getCenter();
            glm::vec3 size = aabb.getSize();

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, center);
            model = glm::scale(model, size);

            glm::mat4 mvp = viewProjectionMatrix * model;

            glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, &mvp[0][0]);
            glUniform3fv(colorLocation, 1, &color[0]);

            glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
        }

        glBindVertexArray(0);
    }
};

Model *model;
Model *model2;
Model *model3;
std::vector<Model*> models;
std::vector<AABB*> boxes;
void load_models() {

  std::vector<glm::mat4> instances;

  model = load_model(resources::path(resources::Models_ford));
  models.push_back(model);
  setupModel(model, 1);
  auto t = model->aabb.getScaleToLengthTransform(5);
  model->aabb = model->aabb.transform(t);
  boxes.push_back(&model->aabb);
  // for(auto& aabb : model->aabbs)  {
  //     aabb = aabb.transform(t);
  // }

  instances.push_back(t);
  uploadInstanceData(model, instances);
  instances.clear();
 //  model2= load_model(resources::path(resources::Models_mustang));
 //  setupModel(model2, 1);
 //  auto tt  = glm::translate(glm::mat4(1), glm::vec3(1, 0,0));
 //  t = model2->aabb.getScaleToLengthTransform(1);
 //  t = glm::rotate(t, (float)M_PI/2, glm::vec3(1, 0,0));
 //  model2->aabb = model2->aabb.transform(tt * t);
 //  instances.push_back(tt  * t);
 //  uploadInstanceData(model2, instances);
 // /*------ */
 //  instances.clear();
 //  model3= load_model(resources::path(resources::Models_pigeon));
 //  setupModel(model3, 1);
 //  auto ttt = glm::translate(glm::mat4(1), glm::vec3(2, 0,0));
 //  t = model3->aabb.getScaleToLengthTransform(1);
 //  // t = glm::rotate(t, (float)M_PI/2, glm::vec3(0, 0,0));

 //  model2->aabb = model2->aabb.transform(ttt * t);
 //  instances.push_back(ttt  * t);
 //  uploadInstanceData(model3, instances);


}
AABBRenderer * aabbRenderer;
unsigned int PROG = 1;
void game_init(GLFWwindow *window) {
    load_models();
    auto ID = Shader::Create(PROG, resources::path(resources::Shaders_vertex), resources::path(resources::Shaders_fragment));
    Shader::Use(ID);
   aabbRenderer = new AABBRenderer();
   ground_system_init();
   collision_system_init();


}






void game_update(float dt) {

    Camera&camera = entt::locator<Camera>::value();
    // ground_system_update(dt);
    collision_system_update(boxes);
    Meta&meta= entt::locator<Meta>::value();
    auto ID = Shader::Get(PROG);
    Shader::Use(ID);
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), meta.WindowDimensions.x / meta.WindowDimensions.y, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    Shader::SetMat4("projection", ID, projection);
    Shader::SetMat4("view", ID, view);
    Shader::SetVec3("viewPos", ID, camera.Position);
    // for()
    drawModel(ID, model, 1);



    // drawModel(ID, model2, 1);
    // drawModel(ID, model3, 1);

        // aabbRenderer->drawAABB(model->aabb, projection * view);

    // for(auto& aabb : model->aabbs){
        // aabbRenderer->drawAABB(aabb, projection * view);
    // }
        // aabbRenderer->drawAABB(model2->aabb, projection * view);
        // aabbRenderer->drawAABB(model3->aabb, projection * view);

}
