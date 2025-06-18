
#include "model_loader.hpp"

// Helper function to load a single texture with duplicate checking
static unsigned int loadTexture(const char *path, const std::string &directory,
                                std::vector<Texture> &textures_loaded)
{

    // Check if already loaded (same logic as your current loadMaterialTextures)
    for (const auto &texture : textures_loaded)
    {
        if (std::strcmp(texture.path.data(), path) == 0)
        {
            return texture.id; // Return existing texture ID
        }
    }

    // Load new texture using your existing function
    uint32_t textureID = TextureFromFile(path, directory);

    // Add to loaded textures cache
    Texture texture;
    texture.id = textureID;
    texture.path = path;
    textures_loaded.push_back(texture);

    return textureID;
}

static Material processMaterial(aiMaterial *aiMat, const std::string &directory,
                                std::vector<Texture> &textures_loaded)
{
    Material material = {};

    // Load textures
    if (aiMat->GetTextureCount(aiTextureType_DIFFUSE) > 0)
    {
        aiString path;
        aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &path);
        material.diffuse_texture = loadTexture(path.C_Str(), directory, textures_loaded);
    }

    if (aiMat->GetTextureCount(aiTextureType_SPECULAR) > 0)
    {
        aiString path;
        aiMat->GetTexture(aiTextureType_SPECULAR, 0, &path);
        material.specular_texture = loadTexture(path.C_Str(), directory, textures_loaded);
    }

    if (aiMat->GetTextureCount(aiTextureType_HEIGHT) > 0)
    {
        aiString path;
        aiMat->GetTexture(aiTextureType_HEIGHT, 0, &path);
        material.normal_texture = loadTexture(path.C_Str(), directory, textures_loaded);
    }

    // Extract material properties from Assimp
    aiColor3D color;

    // Diffuse color
    if (aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
    {
        material.diffuse_color = {color.r, color.g, color.b};
    }
    else
    {
        material.diffuse_color = {1.0f, 1.0f, 1.0f}; // Default white
    }

    // Specular color
    if (aiMat->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS)
    {
        material.specular_color = {color.r, color.g, color.b};
    }
    else
    {
        material.specular_color = {1.0f, 1.0f, 1.0f}; // Default white
    }

    // Shininess (this is the key one you asked about)
    float shininess;
    if (aiMat->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS)
    {
        material.shininess = shininess;
    }
    else
    {
        material.shininess = 32.0f; // Default reasonable value
    }

    // Optional: Other properties if you want them
    float opacity;
    if (aiMat->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS)
    {
        material.alpha = opacity;
    }
    else
    {
        material.alpha = 1.0f;
    }

    // Roughness/metallic aren't standard in older formats,
    // but some exporters put them in custom properties
    float roughness;
    if (aiMat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS)
    {
        material.roughness = roughness;
    }
    else
    {
        // Convert shininess to roughness approximation
        material.roughness = sqrt(2.0f / (shininess + 2.0f));
    }

    float metallic;
    if (aiMat->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS)
    {
        material.metallic = metallic;
    }
    else
    {
        material.metallic = 0.0f; // Default non-metallic
    }

    material.updateTextureFlags();

    return material;
}

static Mesh processMesh(aiMesh *mesh, const aiScene *scene, std::string &directory, std::vector<Texture> &textures_loaded, std::vector<Material> &materials)
{
    std::vector<Vertex> vertices;
    vertices.reserve(mesh->mNumVertices);

    std::vector<unsigned int> indices;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 vector;

        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;

        if (mesh->HasNormals())
        {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
        }

        if (mesh->mTextureCoords[0])
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
        }
        else
        {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    aiMaterial *aiMat = scene->mMaterials[mesh->mMaterialIndex];
    Material material = processMaterial(aiMat, directory, textures_loaded);

    // Store material and get its index
    materials.push_back(material);
    uint32_t material_index = materials.size() - 1;

    Mesh m;
    m.vertices = vertices;
    m.indices = indices;
    m.material_index = material_index;
    return m;
}

static void processNode(aiNode *node, const aiScene *scene, std::string &directory, std::vector<Texture> &textures_loaded, std::vector<Mesh> &meshes, std::vector<Material> &materials)

{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene, directory, textures_loaded, materials));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene, directory, textures_loaded, meshes, materials);
    }
}
Model load_model(std::string path)
{
    Assimp::Importer importer;
    Model model;
    std::vector<Texture> textures_loaded;
    std::string directory;
    bool gammaCorrection;

    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return model;
    }

    directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene, directory, textures_loaded, model.meshes, model.materials);
    return model;
}
