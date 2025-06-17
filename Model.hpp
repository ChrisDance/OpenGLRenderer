#pragma once
#include "mygl.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "texture_utils.hpp"
#include "shader.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <memory>

struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Texture
{
    unsigned int id;
    std::string type;
    std::string path;
};

// Simple, data-oriented approach
struct Material
{
    // Texture handles/IDs (not pointers)
    uint32_t diffuse_texture = 0;
    uint32_t specular_texture = 0;
    uint32_t normal_texture = 0;
    uint32_t roughness_texture = 0;
    uint32_t metallic_texture = 0;

    // Material properties
    glm::vec3 diffuse_color = {1.0f, 1.0f, 1.0f};
    glm::vec3 specular_color = {1.0f, 1.0f, 1.0f};
    float roughness = 0.5f;
    float metallic = 0.0f;
    float alpha = 1.0f;
    float shininess = 32.0f;
};

struct Mesh
{

    unsigned int material_index;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    GLuint VBO, EBO, VAO;
};

struct Model
{
    std::vector<Mesh> meshes;
    std::vector<Material> materials;
    GLuint IVBO; /*instancing*/
    // unsigned int maxInstances{0};
};

struct PointLight
{
    glm::vec3 position;
    float intensity;

    glm::vec3 color;
    float range;

    float constant;
    float linear;
    float quadratic;
    float padding; // Not used, just for alignment
};
