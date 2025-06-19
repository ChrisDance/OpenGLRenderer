#pragma once
#include "mygl.h"
#include "model.hpp"


class AABBRenderer {
private:
  GLuint VAO, VBO, EBO;
  GLuint shaderProgram;
  GLint mvpLocation;
  GLint colorLocation;

  // Cube vertices (unit cube from -0.5 to 0.5)
  static constexpr float vertices[24] = {
      -0.5f, -0.5f, -0.5f, // 0: min corner
      0.5f,  -0.5f, -0.5f, // 1
      -0.5f, 0.5f,  -0.5f, // 2
      0.5f,  0.5f,  -0.5f, // 3
      -0.5f, -0.5f, 0.5f,  // 4
      0.5f,  -0.5f, 0.5f,  // 5
      -0.5f, 0.5f,  0.5f,  // 6
      0.5f,  0.5f,  0.5f   // 7: max corner
  };

  // Wireframe indices (12 edges of a cube)
  static constexpr unsigned int indices[24] = {
      // Bottom face edges
      0, 1, 1, 3, 3, 2, 2, 0,
      // Top face edges
      4, 5, 5, 7, 7, 6, 6, 4,
      // Vertical edges
      0, 4, 1, 5, 2, 6, 3, 7};

  const char *vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        uniform mat4 mvp;
        void main() {
            gl_Position = mvp * vec4(aPos, 1.0);
        }
    )";

  const char *fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec3 color;
        void main() {
            FragColor = vec4(color, 1.0);
        }
    )";

  GLuint compileShader(GLenum type, const char *source) {
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
    GLuint fragmentShader =
        compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

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
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
                 GL_STATIC_DRAW);

    // Set vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);
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
  void drawAABB(const AABB &aabb, const glm::mat4 &viewProjectionMatrix,
                const glm::vec3 &color = glm::vec3(1.0f, 1.0f, 0.0f)) {

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
  void drawAABBs(const std::vector<std::pair<AABB, glm::vec3>> &aabbs,
                 const glm::mat4 &viewProjectionMatrix) {
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);

    for (const auto &[aabb, color] : aabbs) {
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
