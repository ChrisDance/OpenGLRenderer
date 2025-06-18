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

struct DirectionalLight {
    vec3 Direction;
    vec3 Intensity;
    vec3 Color;
};

// Uniform buffer for lighting data
layout (std140) uniform LightingData {
    DirectionalLight directLight;
    int numPointLights;
    PointLight pointLights[MAX_LIGHTS];
    vec3 viewPos;
};

// Uniform buffer for material properties (non-sampler data)
layout (std140) uniform MaterialData {
    vec3 diffuseColor;
    vec3 specularColor;
    float roughness;
    float metallic;
    float shininess;
    bool hasDiffuseTexture;
    bool hasSpecularTexture;
    bool hasNormalMap;
};

// Samplers must remain as regular uniforms (can't be in uniform blocks)
uniform sampler2D material_diffuse;
uniform sampler2D material_specular;
uniform sampler2D material_normalMap;

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
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);

    // Sample specular map if available, otherwise use material color
    vec3 specularValue;
    if (hasSpecularTexture) {
        specularValue = texture(material_specular, TexCoord).rgb;
    } else {
        specularValue = specularColor;
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
    if (hasNormalMap) {
        // Sample normal map and transform from [0,1] to [-1,1]
        vec3 normalMap = texture(material_normalMap, TexCoord).rgb * 2.0 - 1.0;
        // For simplicity, just use the normal map directly (proper implementation would use TBN matrix)
        norm = normalize(Normal + normalMap * 0.1); // Subtle normal mapping
    } else {
        norm = normalize(Normal);
    }

    vec3 viewDirection = normalize(viewPos - FragPos);
    vec3 result = vec3(0.0);

    // Calculate contribution from each point light
    for(int i = 0; i < numPointLights && i < MAX_LIGHTS; i++) {
        result += calculatePointLight(pointLights[i], FragPos, norm, viewDirection);
    }

    // Sample diffuse texture if available, otherwise use material color
    vec3 materialDiffuse;
    if (hasDiffuseTexture) {
        materialDiffuse = texture(material_diffuse, TexCoord).rgb;
    } else {
        materialDiffuse = diffuseColor;
    }

    // Apply lighting to diffuse color
    result = directLight.Color; //(ambient + result) * materialDiffuse;
    FragColor = vec4(result, 1.0);
}
