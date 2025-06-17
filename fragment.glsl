// #version 330 core
// out vec4 FragColor;

// in vec3 FragPos;
// in vec3 Normal;
// in vec2 TexCoord;

// uniform vec3 lightPos;
// uniform vec3 viewPos;
// uniform vec3 lightColor;
// uniform vec3 objectColor;

// // Texture uniforms (for model rendering)
// uniform sampler2D texture_diffuse1;
// uniform sampler2D texture_specular1;

// void main()
// {
//     // Basic Phong lighting
    
//     // Ambient
//     float ambientStrength = 0.1;
//     vec3 ambient = ambientStrength * lightColor;
    
//     // Diffuse
//     vec3 norm = normalize(Normal);
//     vec3 lightDir = normalize(lightPos - FragPos);
//     float diff = max(dot(norm, lightDir), 0.0);
//     vec3 diffuse = diff * lightColor;
    
//     // Specular
//     float specularStrength = 0.5;
//     vec3 viewDir = normalize(viewPos - FragPos);
//     vec3 reflectDir = reflect(-lightDir, norm);
//     float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
//     vec3 specular = specularStrength * spec * lightColor;
    
//     vec3 result = (ambient + diffuse + specular) * objectColor;
    
//     // If using textures (uncomment when loading models with textures):
//     // vec3 diffuseColor = texture(texture_diffuse1, TexCoord).rgb;
//     // vec3 specularColor = texture(texture_specular1, TexCoord).rgb;
//     // result = (ambient + diffuse + specular) * diffuseColor;
    
//     FragColor = vec4(result, 1.0);
// }

#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

// Texture uniforms (for model rendering)
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

void main()
{
    // Basic Phong lighting
    
    // Ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
    
    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;
    
    vec3 result;
    
    // Check if we have a diffuse texture
    vec3 diffuseColor = texture(texture_diffuse1, TexCoord).rgb;
    
    // If the texture is mostly black/empty, use the object color instead
    if (length(diffuseColor) < 0.1) {
        diffuseColor = objectColor;
    }
    
    result = (ambient + diffuse + specular) * diffuseColor;
    
    FragColor = vec4(result, 1.0);
}