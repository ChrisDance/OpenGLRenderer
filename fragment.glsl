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

uniform int numPointLights;
uniform PointLight pointLights[MAX_LIGHTS];
uniform vec3 viewPos;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

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
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0); // 64 = shininess
    
    // Sample specular map for per-pixel specular intensity/color
    vec3 specularMap = texture(texture_specular1, TexCoord).rgb;
    vec3 specular = spec * light.color * specularMap;
    
    // Combine results with attenuation
    return (diffuse + specular) * attenuation;
}

void main()
{
    // Ambient lighting
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * vec3(1.0);

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    vec3 result = vec3(0.0);

    // Calculate contribution from each point light
    for(int i = 0; i < numPointLights && i < MAX_LIGHTS; i++) {
        result += calculatePointLight(pointLights[i], FragPos, norm, viewDir);
    }

    // Sample diffuse texture
    vec3 diffuseColor = texture(texture_diffuse1, TexCoord).rgb;
    
    // Apply lighting to diffuse color
    result = (ambient + result) * diffuseColor;
    
    FragColor = vec4(result, 1.0);
}