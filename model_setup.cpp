#include "model_setup.hpp"

// Create a default white texture (call this once during initialization)
static unsigned int createDefaultTexture() {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    unsigned char whitePixel[] = {255, 255, 255, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return textureID;
}

static unsigned int defaultTexture = 0; // Initialize this once in your setup

static void bindMaterial(Material &material, unsigned int shader)
{
    if (defaultTexture == 0) {
        defaultTexture = createDefaultTexture();
    }
    if(material.diffuse_texture == 0)
    {
        material.hasDiffuseTexture = 0;
        // material.diffuse_color = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
    }
    Shader::setUniformBuffer("MaterialData", shader, &material, material.gpuSize(), 1);

    // Always set sampler uniforms to known texture units
    Shader::setInt("material_diffuse", shader, 0);
    Shader::setInt("material_specular", shader, 1);
    Shader::setInt("material_normalMap", shader, 2);

    // Bind diffuse texture or default
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, material.diffuse_texture != 0 ? material.diffuse_texture : defaultTexture);

    // Bind specular texture or default
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, material.specular_texture != 0 ? material.specular_texture : defaultTexture);

    // Bind normal texture or default
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, material.normal_texture != 0 ? material.normal_texture : defaultTexture);
}



static void setupMesh(Mesh *mesh)
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

static void setupInstanceBuffer(Model *model, int maxInstances)
{
    glGenBuffers(1, &model->IVBO);
    glBindBuffer(GL_ARRAY_BUFFER, model->IVBO);

    // Pre-allocate buffer for maximum instances
    glBufferData(GL_ARRAY_BUFFER,
                 maxInstances * sizeof(glm::mat4),
                 nullptr,
                 GL_DYNAMIC_DRAW);
}

static void setupInstanceAttributes(Model *model, Mesh *mesh)
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

/* exposed */

void uploadInstanceData(Model *model, std::vector<glm::mat4> instances)
{
    size_t dataSize = instances.size() * sizeof(glm::mat4);

    glBindBuffer(GL_ARRAY_BUFFER, model->IVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, instances.data());
}

void drawModel(unsigned int shader, Model *model, unsigned int instanceCount)
{ // if(visibleInstances.empty()) return;

    // Shader::use(shader); // should already be in use

    for (size_t i = 0; i < model->meshes.size(); i++)
    {
        const Mesh &mesh = model->meshes[i];
        Material &material = model->materials[mesh.material_index];
        bindMaterial(material, shader);

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

void setupModel(Model *model, int maxInstances)
{
    // Setup geometry for each mesh
    for (auto &mesh : model->meshes)
    {
        setupMesh(&mesh);
    }

    // Create instance buffer
    setupInstanceBuffer(model, maxInstances);

    // Add instance attributes to each mesh
    for (auto &mesh : model->meshes)
    {
        setupInstanceAttributes(model, &mesh);
    }
}
