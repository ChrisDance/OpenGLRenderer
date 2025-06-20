#pragma once
#include "model.hpp"

class ModelFactory {
public:
    static Model createPlane(float width = 2.0f, float height = 2.0f) {
        Model model;

        // Create vertices for a plane (centered at origin, facing +Z)
        std::vector<Vertex> vertices = {
            // Position                    Normal           TexCoords
            {{-width/2, -height/2, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}, // Bottom-left
            {{ width/2, -height/2, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, // Bottom-right
            {{ width/2,  height/2, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, // Top-right
            {{-width/2,  height/2, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}  // Top-left
        };

        // Create indices for two triangles
        std::vector<unsigned int> indices = {
            0, 1, 2, // First triangle
            2, 3, 0  // Second triangle
        };

        // Create mesh
        Mesh mesh;
        mesh.vertices = vertices;
        mesh.indices = indices;
        mesh.material_index = 0;

        // Setup OpenGL buffers
        // setupMeshBuffers(mesh);

        // Create default material
        Material defaultMaterial;

        // Calculate AABB
        AABB meshAABB;
        for (const auto& vertex : vertices) {
            meshAABB.expand(vertex.Position);
        }

        // Add to model
        model.meshes.push_back(mesh);
        model.materials.push_back(defaultMaterial);
        model.aabbs.push_back(meshAABB);
        model.aabb = meshAABB;

        return model;
    }

    static Model createCube(float size = 1.0f) {
        Model model;
        float half = size / 2.0f;

        // Create vertices for a cube (24 vertices - 4 per face for proper normals)
        std::vector<Vertex> vertices = {
            // Front face (+Z)
            {{-half, -half,  half}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{ half, -half,  half}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{ half,  half,  half}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-half,  half,  half}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},

            // Back face (-Z)
            {{ half, -half, -half}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
            {{-half, -half, -half}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
            {{-half,  half, -half}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
            {{ half,  half, -half}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},

            // Left face (-X)
            {{-half, -half, -half}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{-half, -half,  half}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-half,  half,  half}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{-half,  half, -half}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},

            // Right face (+X)
            {{ half, -half,  half}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{ half, -half, -half}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ half,  half, -half}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{ half,  half,  half}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},

            // Bottom face (-Y)
            {{-half, -half, -half}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
            {{ half, -half, -half}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
            {{ half, -half,  half}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
            {{-half, -half,  half}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},

            // Top face (+Y)
            {{-half,  half,  half}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{ half,  half,  half}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{ half,  half, -half}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
            {{-half,  half, -half}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}
        };

        // Create indices for 12 triangles (2 per face)
        std::vector<unsigned int> indices = {
            // Front face
            0, 1, 2,   2, 3, 0,
            // Back face
            4, 5, 6,   6, 7, 4,
            // Left face
            8, 9, 10,  10, 11, 8,
            // Right face
            12, 13, 14, 14, 15, 12,
            // Bottom face
            16, 17, 18, 18, 19, 16,
            // Top face
            20, 21, 22, 22, 23, 20
        };

        // Create mesh
        Mesh mesh;
        mesh.vertices = vertices;
        mesh.indices = indices;
        mesh.material_index = 0;

        // Setup OpenGL buffers
        // setupMeshBuffers(mesh);

        // Create default material
        Material defaultMaterial;

        // Calculate AABB
        AABB meshAABB(glm::vec3(-half), glm::vec3(half));

        // Add to model
        model.meshes.push_back(mesh);
        model.materials.push_back(defaultMaterial);
        model.aabbs.push_back(meshAABB);
        model.aabb = meshAABB;

        return model;
    }

private:
    static void setupMeshBuffers(Mesh& mesh) {
        // Generate buffers
        glGenVertexArrays(1, &mesh.VAO);
        glGenBuffers(1, &mesh.VBO);
        glGenBuffers(1, &mesh.EBO);

        // Bind VAO
        glBindVertexArray(mesh.VAO);

        // Upload vertex data
        glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
        glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex),
                     mesh.vertices.data(), GL_STATIC_DRAW);

        // Upload index data
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int),
                     mesh.indices.data(), GL_STATIC_DRAW);

        // Set vertex attributes
        // Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

        // Normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (void*)offsetof(Vertex, Normal));

        // Texture coordinates
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (void*)offsetof(Vertex, TexCoords));

        // Unbind VAO
        glBindVertexArray(0);
    }
};
