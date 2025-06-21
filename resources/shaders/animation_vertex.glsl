#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in ivec4 aBoneIDs;
layout(location = 4) in vec4 aWeights;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 bones[100];

out vec3 FragPos;
out vec3 Normal;

void main() {
    mat4 boneTransform = bones[aBoneIDs[0]] * aWeights[0];
    boneTransform += bones[aBoneIDs[1]] * aWeights[1];
    boneTransform += bones[aBoneIDs[2]] * aWeights[2];
    boneTransform += bones[aBoneIDs[3]] * aWeights[3];

    vec4 pos = boneTransform * vec4(aPos, 1.0);
    FragPos = vec3(model * pos);
    Normal = mat3(transpose(inverse(model * boneTransform))) * aNormal;

    gl_Position = projection * view * model * pos;
}
