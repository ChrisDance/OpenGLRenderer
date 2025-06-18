#pragma once
#include "mygl.h"
#include "shader.hpp"
#include "stb_image.h"
#include "texture_utils.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cstddef>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#define MAX_LIGHTS 32

struct Vertex {
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoords;
};

struct Texture {
  unsigned int id;
  std::string type;
  std::string path;
};

// Simple, data-oriented approach
struct Material {
  // GPU-compatible data (matches std140 layout)
  glm::vec3 diffuse_color = {1.0f, 1.0f, 1.0f};
  float _pad1; // vec3 padding for std140
  glm::vec3 specular_color = {1.0f, 1.0f, 1.0f};
  float _pad2; // vec3 padding for std140
  float roughness = 0.5f;
  float metallic = 0.0f;
  float alpha = 1.0f;
  float shininess = 32.0f;
  int hasDiffuseTexture = 0; // bool as int for std140
  int hasSpecularTexture = 0;
  int hasNormalMap = 0;
  float _pad3; // alignment padding

  // CPU-only data (not uploaded to GPU)
  uint32_t diffuse_texture = 0;
  uint32_t specular_texture = 0;
  uint32_t normal_texture = 0;
  uint32_t roughness_texture = 0;
  uint32_t metallic_texture = 0;

  // Helper methods
  void updateTextureFlags() {
    hasDiffuseTexture = (diffuse_texture != 0) ? 1 : 0;
    hasSpecularTexture = (specular_texture != 0) ? 1 : 0;
    hasNormalMap = (normal_texture != 0) ? 1 : 0;
  }

  size_t gpuSize() { return offsetof(Material, diffuse_texture); }
};

struct Mesh {

  unsigned int material_index;
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  GLuint VBO, EBO, VAO;
};

struct Model {
  std::vector<Mesh> meshes;
  std::vector<Material> materials;
  GLuint IVBO; /*instancing*/
               // unsigned int maxInstances{0};
};

struct PointLight {
  glm::vec3 position;
  float _pad1;
  glm::vec3 color;
  float _pad2;
  float constant;
  float linear;
  float quadratic;
  float _pad3;
};

struct DirectionalLight {
  glm::vec3 Direction;
  float _pad1;
  glm::vec3 Intensity;
  float _pad2;
  glm::vec3 Color;
  float _pad3;
};

struct LightingData {
  DirectionalLight directLight;
  int numPointLights;
  float _pad1[3]; // align array to 16 bytes
  PointLight pointLights[MAX_LIGHTS];
  glm::vec3 viewPos;
  float _pad2;
};
