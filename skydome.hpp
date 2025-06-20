#ifndef SKYDOME_H
#define SKYDOME_H


#include <string>
#include "mygl.h"
#include <iostream>
#include <cmath>
#include "lib/stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <cmath>






class SkyDome {
private:
    GLuint textureID;
    GLuint VAO, VBO, EBO;
    GLuint shaderProgram;
    int indexCount;

    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec2 aTexCoord;

        out vec2 TexCoords;

        uniform mat4 projection;
        uniform mat4 view;

        void main() {
            TexCoords = aTexCoord;
            vec4 pos = projection * view * vec4(aPos, 1.0);
            gl_Position = pos.xyww;
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;

        in vec2 TexCoords;

        uniform sampler2D skyTexture;

        void main() {
            FragColor = texture(skyTexture, TexCoords);
        }
    )";

    GLuint compileShader(const char* source, GLenum type) {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            std::cerr << "Shader compilation failed: " << infoLog << std::endl;
        }
        return shader;
    }

    GLuint createShaderProgram() {
        GLuint vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
        GLuint fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);

        GLuint program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(program, 512, nullptr, infoLog);
            std::cerr << "Shader program linking failed: " << infoLog << std::endl;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return program;
    }

    GLuint loadTexture(const std::string& path) {
        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Set texture wrapping/filtering options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int width, height, nrChannels;
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            GLenum format = GL_RGB;
            if (nrChannels == 1) format = GL_RED;
            else if (nrChannels == 3) format = GL_RGB;
            else if (nrChannels == 4) format = GL_RGBA;

            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        } else {
            std::cerr << "Failed to load texture: " << path << std::endl;
        }
        stbi_image_free(data);

        return textureID;
    }

    void generateDome(float radius, int rings, int sectors) {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        // Generate vertices
        for (int ring = 0; ring <= rings; ++ring) {
            float phi = M_PI * 0.5f * ring / rings; // 0 to PI/2 (hemisphere)
            float y = radius * cos(phi);
            float ringRadius = radius * sin(phi);

            for (int sector = 0; sector <= sectors; ++sector) {
                float theta = 2.0f * M_PI * sector / sectors;
                float x = ringRadius * cos(theta);
                float z = ringRadius * sin(theta);

                // Position
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);

                // Texture coordinates
                float u = (float)sector / sectors;
                float v = (float)ring / rings;
                vertices.push_back(u);
                vertices.push_back(v);
            }
        }

        // Generate indices
        for (int ring = 0; ring < rings; ++ring) {
            for (int sector = 0; sector < sectors; ++sector) {
                int current = ring * (sectors + 1) + sector;
                int next = current + sectors + 1;

                // First triangle
                indices.push_back(current);
                indices.push_back(next);
                indices.push_back(current + 1);

                // Second triangle
                indices.push_back(current + 1);
                indices.push_back(next);
                indices.push_back(next + 1);
            }
        }

        indexCount = indices.size();

        // Setup VAO, VBO, EBO
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Texture coordinate attribute
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

public:
    // Constructor takes a single sky texture path
    SkyDome(const std::string& skyTexturePath, float radius = 100.0f, int rings = 16, int sectors = 32) {
        textureID = loadTexture(skyTexturePath);
        shaderProgram = createShaderProgram();
        generateDome(radius, rings, sectors);
    }

    ~SkyDome() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        glDeleteTextures(1, &textureID);
        glDeleteProgram(shaderProgram);
    }

    // Render function - takes glm matrices
    void render(const glm::mat4& view, const glm::mat4& projection) {
        // Change depth function so depth test passes when values are equal to depth buffer's content
        glDepthFunc(GL_LEQUAL);

        glUseProgram(shaderProgram);

        // Remove translation from view matrix (keep only rotation)
        glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(view));

        // Set uniforms
        GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewNoTranslation));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Bind texture and VAO
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glUniform1i(glGetUniformLocation(shaderProgram, "skyTexture"), 0);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Set depth function back to default
        glDepthFunc(GL_LESS);
    }
};

#endif // SKYDOME_H
