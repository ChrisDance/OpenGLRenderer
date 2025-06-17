#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

#define MAX_LIGHTS 32

struct PointLight {
    vec3 position;
    vec3 color;
    float constant;
    float linear;
    float quadratic;
};

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D normalMap;
    
    vec3 diffuseColor;
    vec3 specularColor;
    float roughness;
    float metallic;
    float shininess;
    
    bool hasDiffuseTexture;
    bool hasSpecularTexture;
    bool hasNormalMap;
};

uniform int numPointLights;
uniform PointLight pointLights[MAX_LIGHTS];
uniform vec3 viewPos;
uniform Material material;

vec3 calculatePointLight(PointLight light, vec3 fragPos, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    
    // Calculate distance for attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color;
    
    // Specular shading (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    
    // Sample specular map if available, otherwise use material color
    vec3 specularValue;
    if (material.hasSpecularTexture) {
        specularValue = texture(material.specular, TexCoord).rgb;
    } else {
        specularValue = material.specularColor;
    }
    
    vec3 specular = spec * light.color * specularValue;
    
    // Combine results with attenuation
    return (diffuse + specular) * attenuation;
}

void main()
{
    // Ambient lighting
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * vec3(1.0);
    
    // Use normal map if available
    vec3 norm;
    if (material.hasNormalMap) {
        // Sample normal map and transform from [0,1] to [-1,1]
        vec3 normalMap = texture(material.normalMap, TexCoord).rgb * 2.0 - 1.0;
        // For simplicity, just use the normal map directly (proper implementation would use TBN matrix)
        norm = normalize(Normal + normalMap * 0.1); // Subtle normal mapping
    } else {
        norm = normalize(Normal);
    }
    
    vec3 viewDir = normalize(viewPos - FragPos);
    
    vec3 result = vec3(0.0);
    // Calculate contribution from each point light
    for(int i = 0; i < numPointLights && i < MAX_LIGHTS; i++) {
        result += calculatePointLight(pointLights[i], FragPos, norm, viewDir);
    }
    
    // Sample diffuse texture if available, otherwise use material color
    vec3 diffuseColor;
    if (material.hasDiffuseTexture) {
        diffuseColor = texture(material.diffuse, TexCoord).rgb;
    } else {
        diffuseColor = material.diffuseColor;
    }
    
    // Apply lighting to diffuse color
    result = (ambient + result) * diffuseColor;
    
    FragColor = vec4(result, 1.0);
}