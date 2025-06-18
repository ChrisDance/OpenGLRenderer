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

// Replace uniform buffer blocks with regular uniforms
// Lighting uniforms
uniform DirectionalLight directLight;
uniform int numPointLights;
uniform PointLight pointLights[MAX_LIGHTS];

// Material uniforms
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform float roughness;
uniform float metallic;
uniform float alpha;
uniform float shininess;
uniform int hasDiffuseTexture;
uniform int hasSpecularTexture;
uniform int hasNormalMap;

uniform vec3 viewPos;

// Samplers as regular uniforms
uniform sampler2D material_diffuse;
uniform sampler2D material_specular;
uniform sampler2D material_normalMap;

// Rest of the shader stays exactly the same
vec3 calculatePointLight(PointLight light, vec3 fragPos, vec3 normal, vec3 viewDir, vec3 materialDiffuse)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color * materialDiffuse;

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    vec3 specularValue;
    if (hasSpecularTexture != 0) {
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

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.Color * light.Intensity * materialDiffuse;

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    vec3 specularValue;
    if (hasSpecularTexture != 0) {
        specularValue = texture(material_specular, TexCoord).rgb;
    } else {
        specularValue = specularColor;
    }
    vec3 specular = spec * light.Color * light.Intensity * specularValue;

    return diffuse + specular;
}

void main()
{
    vec3 materialDiffuse;
    if (hasDiffuseTexture != 0) {
        materialDiffuse = texture(material_diffuse, TexCoord).rgb;
    } else {
        materialDiffuse = diffuseColor;
    }

    vec3 norm;
    if (hasNormalMap != 0) {
        vec3 normalMap = texture(material_normalMap, TexCoord).rgb * 2.0 - 1.0;
        norm = normalize(Normal + normalMap * 0.1);
    } else {
        norm = normalize(Normal);
    }

    vec3 viewDirection = normalize(viewPos - FragPos);

    vec3 ambient = 0.3 * materialDiffuse;

    vec3 result = vec3(0.0);
    // Uncomment these when you want lighting back:
    result += calculateDirectionalLight(directLight, norm, viewDirection, materialDiffuse);
    for (int i = 0; i < numPointLights && i < MAX_LIGHTS; i++) {
        result += calculatePointLight(pointLights[i], FragPos, norm, viewDirection, materialDiffuse);
    }

    result += ambient;
    FragColor = vec4(result, 1.0);
}
