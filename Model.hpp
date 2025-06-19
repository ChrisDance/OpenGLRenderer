#pragma once
#include "mygl.h"
#include "shader.hpp"
#include "lib/stb_image.h"
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
#include <entt/entt.hpp>

#define MAX_LIGHTS 32

extern entt::registry ecs;
extern entt::dispatcher bus;




struct AABB {
    glm::vec3 min;
    glm::vec3 max;

    AABB() : min(FLT_MAX), max(-FLT_MAX) {}

    AABB(const glm::vec3& minPoint, const glm::vec3& maxPoint)
        : min(minPoint), max(maxPoint) {}

    // Expand AABB to include a point
    void expand(const glm::vec3& point) {
        min = glm::min(min, point);
        max = glm::max(max, point);
    }

    // Expand AABB to include another AABB
    void expand(const AABB& other) {
        min = glm::min(min, other.min);
        max = glm::max(max, other.max);
    }

    // Get center point
    glm::vec3 getCenter() const {
        return (min + max) * 0.5f;
    }

    // Get size (dimensions)
    glm::vec3 getSize() const {
        return max - min;
    }

    // Get all 8 corners of the AABB
    std::vector<glm::vec3> getCorners() const {
        return {
            glm::vec3(min.x, min.y, min.z),
            glm::vec3(max.x, min.y, min.z),
            glm::vec3(min.x, max.y, min.z),
            glm::vec3(max.x, max.y, min.z),
            glm::vec3(min.x, min.y, max.z),
            glm::vec3(max.x, min.y, max.z),
            glm::vec3(min.x, max.y, max.z),
            glm::vec3(max.x, max.y, max.z)
        };
    }

    // Transform AABB by a matrix
    AABB transform(const glm::mat4& matrix) const {
        auto corners = getCorners();
        AABB result;

        for (const auto& corner : corners) {
            glm::vec4 transformedCorner = matrix * glm::vec4(corner, 1.0f);
            result.expand(glm::vec3(transformedCorner));
        }

        return result;
    }

    // Check if point is inside AABB
    bool contains(const glm::vec3& point) const {
        return point.x >= min.x && point.x <= max.x &&
               point.y >= min.y && point.y <= max.y &&
               point.z >= min.z && point.z <= max.z;
    }

    // Check if two AABBs intersect
    bool intersects(const AABB& other) const {
        return min.x <= other.max.x && max.x >= other.min.x &&
               min.y <= other.max.y && max.y >= other.min.y &&
               min.z <= other.max.z && max.z >= other.min.z;
    }


    // Get scaled AABB with maximum side length of 1
    AABB getUnitScaled() const {
        glm::vec3 size = getSize();
        float maxDimension = glm::max(glm::max(size.x, size.y), size.z);

        if (maxDimension == 0.0f) {
            // Handle degenerate case where AABB has no volume
            return *this;
        }

        float scale = 1.0f / maxDimension;
        glm::vec3 center = getCenter();
        glm::vec3 scaledSize = size * scale;
        glm::vec3 halfScaledSize = scaledSize * 0.5f;

        return AABB(center - halfScaledSize, center + halfScaledSize);
    }


    glm::mat4 getScaleToLengthTransform(float targetLength) const {
        glm::vec3 size = getSize();
        float maxDimension = glm::max(glm::max(size.x, size.y), size.z);

        if (maxDimension == 0.0f) {
            // Handle degenerate case
            return glm::mat4(1.0f); // Identity matrix
        }

        float scale = targetLength / maxDimension;
        return glm::scale(glm::mat4(1.0f), glm::vec3(scale));
    }

};

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
  AABB aabb;

               // unsigned int maxInstances{0};
};
struct PointLight {
    glm::vec3 position;
    glm::vec3 color;
    float constant;
    float linear;
    float quadratic;
    // Remove _pad variables
};

struct DirectionalLight {
    glm::vec3 Direction;
    glm::vec3 Intensity;
    glm::vec3 Color;
    // Remove _pad variables
};

struct LightingData {
    DirectionalLight directLight;
    int numPointLights;
    PointLight pointLights[MAX_LIGHTS];
    // Remove _pad arrays
};


/* storage */

struct Meta
{
    glm::vec2 WindowDimensions;
};
