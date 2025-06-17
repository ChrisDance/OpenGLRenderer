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

// Texture structure
struct Texture
{
    unsigned int id;
    std::string type;
    std::string path;
};

struct Mesh_
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    GLuint VBO, EBO, VAO;
};

struct Model_
{
    std::vector<Mesh_> meshes;
    GLuint IVBO;             /*instancing*/
    glm::mat4 *mappedBuffer; /*Direct pointer to GPU memory*/
    unsigned int maxInstances = 2;
    unsigned int currentCount = 0;
};
