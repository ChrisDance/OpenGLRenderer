#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

void main() {
    vec3 norm = normalize(Normal);
    vec3 color = vec3(0.8, 0.8, 0.9);

    // Simple ambient + diffuse shading
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(0.8, 0.8, 0.8);
    vec3 ambient = 0.3 * color;

    FragColor = vec4(ambient + diffuse * color, 1.0);
}
