// particle_vertex.glsl
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aWorldPos;
layout(location = 3) in vec4 aColor;
layout(location = 4) in float aSize;
layout(location = 5) in float aRotation;

out vec2 TexCoord;
out vec4 ParticleColor;

uniform mat4 view;
uniform mat4 projection;

void main() {
    // Billboard the particle to face the camera
    vec3 CameraRight = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 CameraUp = vec3(view[0][1], view[1][1], view[2][1]);

    // Apply rotation
    float cosR = cos(aRotation);
    float sinR = sin(aRotation);
    vec3 rotatedPos = vec3(
            aPos.x * cosR - aPos.y * sinR,
            aPos.x * sinR + aPos.y * cosR,
            0.0
        );

    // Scale and position the particle
    vec3 worldPos = aWorldPos +
            (CameraRight * rotatedPos.x * aSize) +
            (CameraUp * rotatedPos.y * aSize);

    gl_Position = projection * view * vec4(worldPos, 1.0);

    TexCoord = aTexCoord;
    ParticleColor = aColor;
}
