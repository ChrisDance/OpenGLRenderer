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
layout(std140) uniform LightingData {
    DirectionalLight directLight;
    int numPointLights;
    PointLight pointLights[MAX_LIGHTS];
};

// Uniform buffer for material properties
layout(std140) uniform MaterialData {
    vec3 diffuseColor;
    vec3 specularColor;
    float roughness;
    float metallic;
    float shininess;
    bool hasDiffuseTexture;
    bool hasSpecularTexture;
    bool hasNormalMap;
};

uniform vec3 viewPos;

// Samplers as regular uniforms
uniform sampler2D material_diffuse;
uniform sampler2D material_specular;
uniform sampler2D material_normalMap;

vec3 calculatePointLight(PointLight light, vec3 fragPos, vec3 normal, vec3 viewDir, vec3 materialDiffuse)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color * materialDiffuse;

    // Specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);

    vec3 specularValue;
    if (hasSpecularTexture) {
        specularValue = texture(material_specular, TexCoord).rgb;
    } else {
        specularValue = specularColor;
    }
    vec3 specular = spec * light.color * specularValue;

    return (diffuse + specular) * attenuation;
}

vec3 calculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 materialDiffuse)
{
    vec3 lightDir = normalize(-light.Direction);

    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.Color * light.Intensity * materialDiffuse;

    // Specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);

    vec3 specularValue;
    if (hasSpecularTexture) {
        specularValue = texture(material_specular, TexCoord).rgb;
    } else {
        specularValue = specularColor;
    }
    vec3 specular = spec * light.Color * light.Intensity * specularValue;

    return diffuse + specular;
}

void main()
{
    // Sample diffuse texture if available
    vec3 materialDiffuse;
    if (hasDiffuseTexture) {
        materialDiffuse = texture(material_diffuse, TexCoord).rgb;
    } else {
        materialDiffuse = diffuseColor;
    }

    // Get normal (with optional normal mapping)
    vec3 norm;
    if (hasNormalMap) {
        vec3 normalMap = texture(material_normalMap, TexCoord).rgb * 2.0 - 1.0;
        norm = normalize(Normal + normalMap * 0.1);
    } else {
        norm = normalize(Normal);
    }

    vec3 viewDirection = normalize(viewPos - FragPos);

    // Ambient lighting
    vec3 ambient = 0.1 * materialDiffuse;

    // Directional light
    vec3 result = vec3(0.0);
    result += calculateDirectionalLight(directLight, norm, viewDirection, materialDiffuse);

    // Point lights
    for (int i = 0; i < numPointLights && i < MAX_LIGHTS; i++) {
        result += calculatePointLight(pointLights[i], FragPos, norm, viewDirection, materialDiffuse);
    }

    result += ambient;
    FragColor = vec4(result, 1.0);
}
