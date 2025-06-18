#pragma once
#include "mygl.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <random>

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec4 color;
    float life;
    float size;
    float rotation;
    float rotationSpeed;
};

struct ParticleEmitterConfig {
    // Emission
    float emissionRate = 50.0f;  // particles per second
    int maxParticles = 1000;

    // Spawn properties
    glm::vec3 spawnPosition = glm::vec3(0.0f);
    glm::vec3 spawnPositionVariance = glm::vec3(0.1f);
    glm::vec3 initialVelocity = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 velocityVariance = glm::vec3(0.5f);

    // Particle properties
    float particleLifetime = 2.0f;
    float lifetimeVariance = 0.5f;
    float startSize = 0.1f;
    float endSize = 0.05f;
    float sizeVariance = 0.02f;

    // Color over lifetime
    glm::vec4 startColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    glm::vec4 endColor = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);

    // Physics
    glm::vec3 gravity = glm::vec3(0.0f, -9.81f, 0.0f);
    float damping = 0.98f;

    // Rotation
    float rotationSpeed = 0.0f;
    float rotationVariance = 1.0f;

    // Simulation
    bool looping = true;
    float duration = 5.0f;  // Only used if looping is false
};

class ParticleEmitter {
private:
    std::vector<Particle> particles;
    ParticleEmitterConfig config;

    // OpenGL resources
    GLuint VAO, VBO;

    // Emission timing
    float emissionTimer = 0.0f;
    float systemTime = 0.0f;
    bool isActive = true;

    // Random number generation
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<float> dis;

    // Vertex data for instanced rendering
    struct ParticleVertex {
        glm::vec3 position;
        glm::vec2 texCoord;
    };

    // Instance data uploaded to GPU
    struct ParticleInstanceData {
        glm::vec3 worldPosition;
        glm::vec4 color;
        float size;
        float rotation;
    };

    GLuint instanceVBO;
    std::vector<ParticleInstanceData> instanceData;

    void setupBuffers();
    void spawnParticle();
    float randomFloat(float min, float max);
    glm::vec3 randomVec3(const glm::vec3& base, const glm::vec3& variance);

public:
    ParticleEmitter(const ParticleEmitterConfig& cfg);
    ~ParticleEmitter();

    void update(float deltaTime);
    void render(GLuint shaderProgram, const glm::mat4& view, const glm::mat4& projection);

    // Control methods
    void start() { isActive = true; systemTime = 0.0f; }
    void stop() { isActive = false; }
    void reset();

    // Configuration
    void setConfig(const ParticleEmitterConfig& cfg) { config = cfg; }
    const ParticleEmitterConfig& getConfig() const { return config; }

    // Transform
    void setPosition(const glm::vec3& pos) { config.spawnPosition = pos; }
    glm::vec3 getPosition() const { return config.spawnPosition; }

    // State queries
    bool isAlive() const { return isActive || !particles.empty(); }
    int getParticleCount() const { return particles.size(); }
};

// Utility function to create common emitter presets
namespace ParticlePresets {
    ParticleEmitterConfig createFire();
    ParticleEmitterConfig createSmoke();
    ParticleEmitterConfig createSparks();
    ParticleEmitterConfig createSnow();
    ParticleEmitterConfig createExplosion();
}
