#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

#define MAX_LIGHTS 16

struct PointLight {
    vec3 position;
    float intensity; 
    vec3 color;
    float range;    
    float constant;
    float linear;
    float quadratic;
    float padding;  // Not used, just for alignment
};


#define FLOATS_PER_LIGHT 12  // 12 floats per light struct


uniform vec3 viewPos;
uniform float lightData[MAX_LIGHTS * FLOATS_PER_LIGHT];
uniform int numPointLights;


// Texture uniforms (for model rendering)
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

// Helper function to extract a light from the packed data
PointLight getLight(int index) {
    PointLight light;
    int base = index * FLOATS_PER_LIGHT;
    
    light.position = vec3(lightData[base], lightData[base+1], lightData[base+2]);
    light.intensity = lightData[base+3];
    light.color = vec3(lightData[base+4], lightData[base+5], lightData[base+6]);
    light.range = lightData[base+7];
    light.constant = lightData[base+8];
    light.linear = lightData[base+9];
    light.quadratic = lightData[base+10];
    // lightData[base+11] is padding
    
    return light;
}

vec3 ambient = vec3(0.0f);
vec3 calculatePointLight(PointLight light, vec3 fragPos, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    float distance = length(light.position - fragPos);

    // // Early exit if fragment is outside light range
    // if (distance > light.range) {
    //     // ambient = vec3(1.0);
    //     return vec3(0.0);
    // }
    
    // Attenuation calculation
    float attenuation = 1.0 / (light.constant + 
                              light.linear * distance + 
                              light.quadratic * distance * distance);
    
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color;
    
    // Specular shading (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0); // 64 = shininess
    vec3 specular = spec * light.color;
    
    // Combine results
    return (diffuse + specular) * light.intensity * attenuation;
}

void main()
{

    // Ambient
    float ambientStrength = 0.1;
    ambient = ambientStrength * vec3(1.0);

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    vec3 result = vec3(0.0);

    for(int i = 0; i < numPointLights && i < MAX_LIGHTS; i++) {
        PointLight light = getLight(i);
        result += calculatePointLight(light, FragPos, norm, viewDir);
    }

    
    // Check if we have a diffuse texture
    vec3 diffuseColor = texture(texture_diffuse1, TexCoord).rgb;
    
    // If the texture is mostly black/empty, use the object color instead
    // if (length(diffuseColor) < 0.1) {
    //     diffuseColor = objectColor;
    // }
    
    result = (ambient + result) * diffuseColor;
    
    FragColor = vec4(result, 1.0);
}