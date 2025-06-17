#include "mesh_util.hpp"
void Draw(Mesh_ *mesh, unsigned int Shader)
{
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;

    for (unsigned int i = 0; i < mesh->textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);

        std::string number;
        std::string name = mesh->textures[i].type;
        if (name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if (name == "texture_specular")
            number = std::to_string(specularNr++);
        else if (name == "texture_normal")
            number = std::to_string(normalNr++);
        else if (name == "texture_height")
            number = std::to_string(heightNr++);

        glUniform1i(glGetUniformLocation(Shader, (name + number).c_str()), i);
        glBindTexture(GL_TEXTURE_2D, mesh->textures[i].id);
    }

    glBindVertexArray(mesh->VAO);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh->indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0);
}

void bindMeshTextures(const Mesh_ &mesh, unsigned int shader)

{
    uint32_t diffuseNr = 1;
    uint32_t specularNr = 1;
    uint32_t normalNr = 1;
    uint32_t heightNr = 1;

    for (uint32_t i = 0; i < mesh.textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);

        // Determine texture type and create uniform name
        std::string number;
        std::string name = mesh.textures[i].type;

        if (name == "texture_diffuse")
        {
            number = std::to_string(diffuseNr++);
        }
        else if (name == "texture_specular")
        {
            number = std::to_string(specularNr++);
        }
        else if (name == "texture_normal")
        {
            number = std::to_string(normalNr++);
        }
        else if (name == "texture_height")
        {
            number = std::to_string(heightNr++);
        }

        // Set the sampler uniform

        Shader::setInt((name + number).c_str(), shader, i);
        glBindTexture(GL_TEXTURE_2D, mesh.textures[i].id);
    }
}
void draw(unsigned int shader, Model_ *model, std::vector<glm::mat4> &instances)
{
    // if(visibleInstances.empty()) return;

    uint32_t instanceCount = static_cast<uint32_t>(instances.size());

    // Shader::use(shader); // should already be in use

    for (size_t i = 0; i < model->meshes.size(); i++)
    {
        const Mesh_ &mesh = model->meshes[i];

        bindMeshTextures(mesh, shader);

        // Draw this mesh with all instances
        glBindVertexArray(mesh.VAO);
        glDrawElementsInstanced(GL_TRIANGLES,
                                static_cast<GLsizei>(mesh.indices.size()),
                                GL_UNSIGNED_INT,
                                0,
                                instanceCount);
    }

    glBindVertexArray(0);
}

static void unloadMesh(Mesh_ *mesh)
{
    glDeleteBuffers(1, &mesh->VBO);
    glDeleteBuffers(1, &mesh->EBO);
    glDeleteVertexArrays(1, &mesh->VAO);
}

static void setupMesh(Mesh_ *mesh)
{
    glGenVertexArrays(1, &mesh->VAO);
    glGenBuffers(1, &mesh->VBO);
    glGenBuffers(1, &mesh->EBO);

    glBindVertexArray(mesh->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh->vertices.size() * sizeof(Vertex), &mesh->vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices.size() * sizeof(unsigned int), &mesh->indices[0], GL_STATIC_DRAW);

    // Vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);

    // Vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Normal));

    // Vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, TexCoords));

    glBindVertexArray(0);
}

void uploadInstanceData(Model_ *model, std::vector<glm::mat4> instances)
{
    size_t dataSize = instances.size() * sizeof(glm::mat4);

    glBindBuffer(GL_ARRAY_BUFFER, model->IVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, instances.data());
}

static void setupInstanceBuffer(Model_ *model)
{
    glGenBuffers(1, &model->IVBO);
    glBindBuffer(GL_ARRAY_BUFFER, model->IVBO);

    // Pre-allocate buffer for maximum instances
    glBufferData(GL_ARRAY_BUFFER,
                 model->maxInstances * sizeof(glm::mat4),
                 nullptr,
                 GL_DYNAMIC_DRAW);
}

static void setupInstanceAttributes(Model_ *model, Mesh_ *mesh)
{
    glBindVertexArray(mesh->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, model->IVBO);

    // Instance matrix (mat4 = 4 vec4s, using attributes 3-6)
    for (int i = 0; i < 4; i++)
    {
        glEnableVertexAttribArray(3 + i);
        glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE,
                              sizeof(glm::mat4),
                              (void *)(i * sizeof(glm::vec4)));
        glVertexAttribDivisor(3 + i, 1); // Per-instance data
    }

    glBindVertexArray(0);
}

void setupModel(Model_ *model)
{
    // Setup geometry for each mesh
    for (auto &mesh : model->meshes)
    {
        setupMesh(&mesh);
    }

    // Create instance buffer
    setupInstanceBuffer(model);

    // Add instance attributes to each mesh
    for (auto &mesh : model->meshes)
    {
        setupInstanceAttributes(model, &mesh);
    }
}
