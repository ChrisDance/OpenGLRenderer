#include "particle_emitter.hpp"
#include <algorithm>
#include <cmath>

ParticleEmitter::ParticleEmitter(const ParticleEmitterConfig& cfg)
    : config(cfg), gen(rd()), dis(0.0f, 1.0f) {
    particles.reserve(config.maxParticles);
    instanceData.reserve(config.maxParticles);
    setupBuffers();
}

ParticleEmitter::~ParticleEmitter() {
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &instanceVBO);
    glDeleteVertexArrays(1, &VAO);
}

void ParticleEmitter::setupBuffers() {
    // Create quad vertices for billboarded particles
    ParticleVertex vertices[] = {
        {{ -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f }},
        {{  0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f }},
        {{  0.5f,  0.5f, 0.0f }, { 1.0f, 1.0f }},
        {{ -0.5f,  0.5f, 0.0f }, { 0.0f, 1.0f }}
    };

    GLuint indices[] = { 0, 1, 2, 2, 3, 0 };

    GLuint EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &instanceVBO);

    glBindVertexArray(VAO);

    // Setup vertex data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Vertex attributes (per-vertex data)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex), (void*)offsetof(ParticleVertex, texCoord));

    // Setup instance buffer
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, config.maxParticles * sizeof(ParticleInstanceData), nullptr, GL_DYNAMIC_DRAW);

    // Instance attributes (per-instance data)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData), (void*)0);
    glVertexAttribDivisor(2, 1);

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData), (void*)offsetof(ParticleInstanceData, color));
    glVertexAttribDivisor(3, 1);

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData), (void*)offsetof(ParticleInstanceData, size));
    glVertexAttribDivisor(4, 1);

    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData), (void*)offsetof(ParticleInstanceData, rotation));
    glVertexAttribDivisor(5, 1);

    glBindVertexArray(0);
}

void ParticleEmitter::update(float deltaTime) {
    systemTime += deltaTime;

    // Check if we should stop emitting (for non-looping emitters)
    if (!config.looping && systemTime >= config.duration) {
        isActive = false;
    }

    // Emit new particles
    if (isActive && particles.size() < config.maxParticles) {
        emissionTimer += deltaTime;
        float emissionInterval = 1.0f / config.emissionRate;

        while (emissionTimer >= emissionInterval && particles.size() < config.maxParticles) {
            spawnParticle();
            emissionTimer -= emissionInterval;
        }
    }

    // Update existing particles
    for (auto it = particles.begin(); it != particles.end();) {
        Particle& p = *it;

        // Update physics
        p.velocity += config.gravity * deltaTime;
        p.velocity *= config.damping;
        p.position += p.velocity * deltaTime;
        p.rotation += p.rotationSpeed * deltaTime;

        // Update life
        p.life -= deltaTime;

        if (p.life <= 0.0f) {
            it = particles.erase(it);
        } else {
            ++it;
        }
    }

    // Prepare instance data for rendering
    instanceData.clear();
    for (const auto& particle : particles) {
        float lifeRatio = particle.life / config.particleLifetime;

        ParticleInstanceData data;
        data.worldPosition = particle.position;
        data.color = glm::mix(config.endColor, config.startColor, lifeRatio);
        data.size = glm::mix(config.endSize, config.startSize, lifeRatio) * particle.size;
        data.rotation = particle.rotation;

        instanceData.push_back(data);
    }
}

void ParticleEmitter::render(GLuint shaderProgram, const glm::mat4& view, const glm::mat4& projection) {
    if (instanceData.empty()) return;

    glUseProgram(shaderProgram);

    // Upload matrices
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

    // Upload instance data
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, instanceData.size() * sizeof(ParticleInstanceData), instanceData.data());

    // Render
    glBindVertexArray(VAO);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, instanceData.size());
    glBindVertexArray(0);
}

void ParticleEmitter::reset() {
    particles.clear();
    instanceData.clear();
    emissionTimer = 0.0f;
    systemTime = 0.0f;
    isActive = true;
}

void ParticleEmitter::spawnParticle() {
    Particle p;

    p.position = randomVec3(config.spawnPosition, config.spawnPositionVariance);
    p.velocity = randomVec3(config.initialVelocity, config.velocityVariance);
    p.life = config.particleLifetime + randomFloat(-config.lifetimeVariance, config.lifetimeVariance);
    p.size = config.startSize + randomFloat(-config.sizeVariance, config.sizeVariance);
    p.rotation = randomFloat(0.0f, 2.0f * M_PI);
    p.rotationSpeed = config.rotationSpeed + randomFloat(-config.rotationVariance, config.rotationVariance);
    p.color = config.startColor;

    particles.push_back(p);
}

float ParticleEmitter::randomFloat(float min, float max) {
    return min + dis(gen) * (max - min);
}

glm::vec3 ParticleEmitter::randomVec3(const glm::vec3& base, const glm::vec3& variance) {
    return glm::vec3(
        base.x + randomFloat(-variance.x, variance.x),
        base.y + randomFloat(-variance.y, variance.y),
        base.z + randomFloat(-variance.z, variance.z)
    );
}

// Preset implementations
namespace ParticlePresets {
    ParticleEmitterConfig createFire() {
        ParticleEmitterConfig config;
        config.emissionRate = 100.0f;
        config.maxParticles = 500;
        config.spawnPositionVariance = glm::vec3(0.2f, 0.1f, 0.2f);
        config.initialVelocity = glm::vec3(0.0f, 2.0f, 0.0f);
        config.velocityVariance = glm::vec3(0.5f, 0.5f, 0.5f);
        config.particleLifetime = 1.5f;
        config.startSize = 0.1f;
        config.endSize = 0.3f;
        config.startColor = glm::vec4(1.0f, 0.8f, 0.2f, 1.0f);
        config.endColor = glm::vec4(1.0f, 0.2f, 0.0f, 0.0f);
        config.gravity = glm::vec3(0.0f, 1.0f, 0.0f);
        config.damping = 0.95f;
        return config;
    }

    ParticleEmitterConfig createSmoke() {
        ParticleEmitterConfig config;
        config.emissionRate = 30.0f;
        config.maxParticles = 200;
        config.spawnPositionVariance = glm::vec3(0.1f, 0.0f, 0.1f);
        config.initialVelocity = glm::vec3(0.0f, 1.0f, 0.0f);
        config.velocityVariance = glm::vec3(0.3f, 0.2f, 0.3f);
        config.particleLifetime = 3.0f;
        config.startSize = 0.2f;
        config.endSize = 0.5f;
        config.startColor = glm::vec4(0.3f, 0.3f, 0.3f, 0.8f);
        config.endColor = glm::vec4(0.6f, 0.6f, 0.6f, 0.0f);
        config.gravity = glm::vec3(0.0f, 0.5f, 0.0f);
        config.damping = 0.99f;
        return config;
    }

    ParticleEmitterConfig createSparks() {
        ParticleEmitterConfig config;
        config.emissionRate = 200.0f;
        config.maxParticles = 300;
        config.spawnPositionVariance = glm::vec3(0.05f);
        config.initialVelocity = glm::vec3(0.0f, 3.0f, 0.0f);
        config.velocityVariance = glm::vec3(2.0f, 1.0f, 2.0f);
        config.particleLifetime = 0.8f;
        config.startSize = 0.02f;
        config.endSize = 0.01f;
        config.startColor = glm::vec4(1.0f, 1.0f, 0.5f, 1.0f);
        config.endColor = glm::vec4(1.0f, 0.3f, 0.0f, 0.0f);
        config.gravity = glm::vec3(0.0f, -15.0f, 0.0f);
        config.damping = 0.98f;
        config.looping = false;
        config.duration = 0.5f;
        return config;
    }

    ParticleEmitterConfig createSnow() {
        ParticleEmitterConfig config;
        config.emissionRate = 50.0f;
        config.maxParticles = 1000;
        config.spawnPosition = glm::vec3(0.0f, 10.0f, 0.0f);
        config.spawnPositionVariance = glm::vec3(10.0f, 1.0f, 10.0f);
        config.initialVelocity = glm::vec3(0.0f, -1.0f, 0.0f);
        config.velocityVariance = glm::vec3(0.5f, 0.2f, 0.5f);
        config.particleLifetime = 15.0f;
        config.startSize = 0.05f;
        config.endSize = 0.05f;
        config.startColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        config.endColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        config.gravity = glm::vec3(0.0f, -2.0f, 0.0f);
        config.damping = 1.0f;
        return config;
    }

    ParticleEmitterConfig createExplosion() {
        ParticleEmitterConfig config;
        config.emissionRate = 1000.0f;
        config.maxParticles = 500;
        config.spawnPositionVariance = glm::vec3(0.1f);
        config.initialVelocity = glm::vec3(0.0f, 0.0f, 0.0f);
        config.velocityVariance = glm::vec3(5.0f, 5.0f, 5.0f);
        config.particleLifetime = 2.0f;
        config.startSize = 0.1f;
        config.endSize = 0.0f;
        config.startColor = glm::vec4(1.0f, 0.5f, 0.0f, 1.0f);
        config.endColor = glm::vec4(0.5f, 0.0f, 0.0f, 0.0f);
        config.gravity = glm::vec3(0.0f, -5.0f, 0.0f);
        config.damping = 0.95f;
        config.looping = false;
        config.duration = 0.1f;
        return config;
    }
}
