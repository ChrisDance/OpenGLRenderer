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

struct Mesh
{
    std::vector<Vertex> vertices;
    std::vector<Texture> textures;
    std::vector<unsigned int> indices;
    GLuint VBO, EBO, VAO;
};

struct Model
{
    std::vector<Mesh> meshes;
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
