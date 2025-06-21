
#include "mygl.h"
#include "resource_ids.hpp"
#include "shader.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>
#include <vector>
#include <string>
#include <map>

// Extended vertex structure for animation
struct AnimatedVertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::ivec4 BoneIDs{0};
    glm::vec4 Weights{0.0f};
};

// Bone structure
struct Bone {
    glm::mat4 offset;
    glm::mat4 transformation;
};

// Animation keyframes
struct KeyFrame {
    float time;
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
};

struct BoneAnimation {
    std::vector<KeyFrame> keyframes;
};

struct AnimatedMesh {
    std::vector<AnimatedVertex> vertices;
    std::vector<unsigned int> indices;
    GLuint VAO = 0, VBO = 0, EBO = 0;

    void setupMesh() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(AnimatedVertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(AnimatedVertex), (void*)0);

        // Normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(AnimatedVertex), (void*)offsetof(AnimatedVertex, Normal));

        // Texture coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(AnimatedVertex), (void*)offsetof(AnimatedVertex, TexCoords));

        // Bone IDs
        glEnableVertexAttribArray(3);
        glVertexAttribIPointer(3, 4, GL_INT, sizeof(AnimatedVertex), (void*)offsetof(AnimatedVertex, BoneIDs));

        // Bone weights
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(AnimatedVertex), (void*)offsetof(AnimatedVertex, Weights));

        glBindVertexArray(0);
    }
};

class AnimatedModel {
private:
    std::vector<AnimatedMesh> meshes;
    std::vector<Bone> bones;
    std::map<std::string, int> boneMapping;
    std::map<std::string, BoneAnimation> animations;

    GLuint shaderProgram;

    glm::mat4 globalInverseTransform;
    float animationTime = 0.0f;
    float animationDuration = 0.0f;

public:
    bool loadModel(const std::string& path) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path,
            aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
            return false;
        }

        globalInverseTransform = glm::inverse(aiMatrix4x4ToGlm(scene->mRootNode->mTransformation));

        // Reserve space to prevent reallocation issues
        meshes.reserve(scene->mNumMeshes);
        bones.reserve(100); // Reasonable estimate

        // Process the scene hierarchy like your existing code
        processNode(scene->mRootNode, scene);

        // Load first animation if available
        if (scene->mNumAnimations > 0) {
            loadAnimation(scene->mAnimations[0]);
            std::cout << "Loaded animation with duration: " << animationDuration << " seconds" << std::endl;
        } else {
            std::cout << "No animations found in model" << std::endl;
        }

        std::cout << "Loaded " << meshes.size() << " meshes with " << bones.size() << " bones" << std::endl;

        // Setup all meshes after loading is complete
        for (auto& mesh : meshes) {
            mesh.setupMesh();
        }

        return true;
    }

    void update(float deltaTime) {
        if (animationDuration > 0) {
            animationTime += deltaTime;
            if (animationTime > animationDuration) {
                animationTime = 0.0f; // Loop animation
            }
        }
    }

    void initShaders() {
       shaderProgram = Shader::Create(resources::path(resources::Shaders_animation_vertex), resources::path(resources::Shaders_animation_fragment));
    }

    void render(const glm::mat4& view, const glm::mat4& projection) {
        glUseProgram(shaderProgram);

        // Update bone transformations
        std::vector<glm::mat4> boneTransforms(100, glm::mat4(1.0f)); // Max 100 bones
        calculateBoneTransformations(animationTime, boneTransforms);

        // Set uniforms
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f)); // Try scaling if model is too small
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        // Set bone transforms
        for (size_t i = 0; i < boneTransforms.size(); ++i) {
            std::string name = "bones[" + std::to_string(i) + "]";
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, name.c_str()), 1, GL_FALSE, glm::value_ptr(boneTransforms[i]));
        }

        // Render all meshes
        for (const auto& mesh : meshes) {
            glBindVertexArray(mesh.VAO);
            glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
        }
    }


private:
    glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4& from) {
        glm::mat4 to;
        to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
        to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
        to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
        to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
        return to;
    }

    // Process node hierarchy like your existing code
    void processNode(aiNode* node, const aiScene* scene) {
        // Process all meshes in current node
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.emplace_back(processMesh(mesh, scene));
        }

        // Process child nodes
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene);
        }
    }

    AnimatedMesh processMesh(aiMesh* mesh, const aiScene* scene) {
        AnimatedMesh animMesh;
        animMesh.vertices.reserve(mesh->mNumVertices);

        // Process vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            AnimatedVertex vertex;
            vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

            if (mesh->HasNormals()) {
                vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
            }

            if (mesh->mTextureCoords[0]) {
                vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
            } else {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            }

            animMesh.vertices.push_back(vertex);
        }

        // Process indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                animMesh.indices.push_back(face.mIndices[j]);
            }
        }

        // Process bones
        if (mesh->HasBones()) {
            loadBones(mesh, animMesh);
        }

        return animMesh;
    }

    void loadBones(aiMesh* mesh, AnimatedMesh& animMesh) {
        for (unsigned int i = 0; i < mesh->mNumBones; i++) {
            aiBone* bone = mesh->mBones[i];
            std::string boneName = bone->mName.data;

            int boneIndex;
            if (boneMapping.find(boneName) == boneMapping.end()) {
                boneIndex = bones.size();
                boneMapping[boneName] = boneIndex;
                Bone newBone;
                newBone.offset = aiMatrix4x4ToGlm(bone->mOffsetMatrix);
                bones.push_back(newBone);
            } else {
                boneIndex = boneMapping[boneName];
            }

            for (unsigned int j = 0; j < bone->mNumWeights; j++) {
                aiVertexWeight weight = bone->mWeights[j];
                unsigned int vertexID = weight.mVertexId;

                // Find first available slot
                for (int k = 0; k < 4; k++) {
                    if (animMesh.vertices[vertexID].Weights[k] == 0.0f) {
                        animMesh.vertices[vertexID].BoneIDs[k] = boneIndex;
                        animMesh.vertices[vertexID].Weights[k] = weight.mWeight;
                        break;
                    }
                }
            }
        }
    }

    void loadAnimation(aiAnimation* animation) {
        animationDuration = animation->mDuration / animation->mTicksPerSecond;

        for (unsigned int i = 0; i < animation->mNumChannels; i++) {
            aiNodeAnim* nodeAnim = animation->mChannels[i];
            std::string boneName = nodeAnim->mNodeName.data;

            BoneAnimation boneAnim;
            boneAnim.keyframes.reserve(nodeAnim->mNumPositionKeys);

            // Assume position, rotation, and scaling keys are synchronized
            unsigned int numKeys = nodeAnim->mNumPositionKeys;

            for (unsigned int j = 0; j < numKeys; j++) {
                KeyFrame keyframe;
                keyframe.time = nodeAnim->mPositionKeys[j].mTime / animation->mTicksPerSecond;

                // Position
                keyframe.position = glm::vec3(
                    nodeAnim->mPositionKeys[j].mValue.x,
                    nodeAnim->mPositionKeys[j].mValue.y,
                    nodeAnim->mPositionKeys[j].mValue.z
                );

                // Rotation
                if (j < nodeAnim->mNumRotationKeys) {
                    keyframe.rotation = glm::quat(
                        nodeAnim->mRotationKeys[j].mValue.w,
                        nodeAnim->mRotationKeys[j].mValue.x,
                        nodeAnim->mRotationKeys[j].mValue.y,
                        nodeAnim->mRotationKeys[j].mValue.z
                    );
                } else {
                    keyframe.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
                }

                // Scale
                if (j < nodeAnim->mNumScalingKeys) {
                    keyframe.scale = glm::vec3(
                        nodeAnim->mScalingKeys[j].mValue.x,
                        nodeAnim->mScalingKeys[j].mValue.y,
                        nodeAnim->mScalingKeys[j].mValue.z
                    );
                } else {
                    keyframe.scale = glm::vec3(1.0f);
                }

                boneAnim.keyframes.push_back(keyframe);
            }

            animations[boneName] = std::move(boneAnim);
        }
    }

    void calculateBoneTransformations(float time, std::vector<glm::mat4>& transforms) {
        for (const auto& pair : boneMapping) {
            const std::string& boneName = pair.first;
            int boneIndex = pair.second;

            glm::mat4 boneTransform = glm::mat4(1.0f);

            auto animIt = animations.find(boneName);
            if (animIt != animations.end()) {
                const BoneAnimation& anim = animIt->second;

                if (!anim.keyframes.empty()) {
                    // Find current keyframe
                    int currentFrame = 0;
                    for (size_t i = 0; i < anim.keyframes.size() - 1; i++) {
                        if (time >= anim.keyframes[i].time && time < anim.keyframes[i + 1].time) {
                            currentFrame = i;
                            break;
                        }
                    }

                    if (currentFrame < anim.keyframes.size() - 1) {
                        const KeyFrame& current = anim.keyframes[currentFrame];
                        const KeyFrame& next = anim.keyframes[currentFrame + 1];

                        float factor = (time - current.time) / (next.time - current.time);

                        glm::vec3 pos = glm::mix(current.position, next.position, factor);
                        glm::quat rot = glm::slerp(current.rotation, next.rotation, factor);
                        glm::vec3 scale = glm::mix(current.scale, next.scale, factor);

                        boneTransform = glm::translate(glm::mat4(1.0f), pos) *
                                       glm::mat4_cast(rot) *
                                       glm::scale(glm::mat4(1.0f), scale);
                    } else {
                        const KeyFrame& frame = anim.keyframes[0];
                        boneTransform = glm::translate(glm::mat4(1.0f), frame.position) *
                                       glm::mat4_cast(frame.rotation) *
                                       glm::scale(glm::mat4(1.0f), frame.scale);
                    }
                }
            }

            if (boneIndex < transforms.size()) {
                transforms[boneIndex] = globalInverseTransform * boneTransform * bones[boneIndex].offset;
            }
        }
    }
};
