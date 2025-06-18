#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

#define MAX_LIGHTS 32
#define PI 3.14159265359

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

// Existing uniforms - reinterpreted for PBR
uniform DirectionalLight directLight;
uniform int numPointLights;
uniform PointLight pointLights[MAX_LIGHTS];

// Material uniforms (reinterpreted for PBR)
uniform vec3 diffuseColor; // Used as albedo
uniform vec3 specularColor; // Used as F0 (base reflectance)
uniform float roughness; // Surface roughness [0,1]
uniform float metallic; // Metallic factor [0,1]
uniform float alpha;
uniform float shininess; // Ignored in PBR
uniform int hasDiffuseTexture;
uniform int hasSpecularTexture;
uniform int hasNormalMap;
uniform vec3 viewPos;

uniform sampler2D material_diffuse; // Albedo texture
uniform sampler2D material_specular; // Can be metallic or specular texture
uniform sampler2D material_normalMap;

// PBR Functions

// Normal Distribution Function (GGX/Trowbridge-Reitz)
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

// Geometry Function (Smith's method)
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// Fresnel Function (Schlick's approximation)
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// PBR Point Light calculation
vec3 calculatePBRPointLight(PointLight light, vec3 fragPos, vec3 normal, vec3 viewDir,
    vec3 albedo, float metallicValue, float roughnessValue, vec3 F0) {
    vec3 lightDir = normalize(light.position - fragPos);
    vec3 halfwayDir = normalize(viewDir + lightDir);
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    vec3 radiance = light.color * attenuation;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(normal, halfwayDir, roughnessValue);
    float G = GeometrySmith(normal, viewDir, lightDir, roughnessValue);
    vec3 F = fresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallicValue;

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    float NdotL = max(dot(normal, lightDir), 0.0);
    return (kD * albedo / PI + specular) * radiance * NdotL;
}

// PBR Directional Light calculation
vec3 calculatePBRDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir,
    vec3 albedo, float metallicValue, float roughnessValue, vec3 F0) {
    vec3 lightDir = normalize(-light.Direction);
    vec3 halfwayDir = normalize(viewDir + lightDir);
    vec3 radiance = light.Color * light.Intensity;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(normal, halfwayDir, roughnessValue);
    float G = GeometrySmith(normal, viewDir, lightDir, roughnessValue);
    vec3 F = fresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallicValue;

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    float NdotL = max(dot(normal, lightDir), 0.0);
    return (kD * albedo / PI + specular) * radiance * NdotL;
}

void main() {
    // Get material properties
    vec3 albedo;
    if (hasDiffuseTexture != 0) {
        albedo = texture(material_diffuse, TexCoord).rgb * diffuseColor;
    } else {
        albedo = diffuseColor;
    }

    // Get normal
    vec3 norm;
    if (hasNormalMap != 0) {
        vec3 normalMap = texture(material_normalMap, TexCoord).rgb * 2.0 - 1.0;
        norm = normalize(Normal + normalMap * 0.1);
    } else {
        norm = normalize(Normal);
    }

    vec3 viewDirection = normalize(viewPos - FragPos);

    // PBR material setup
    float metallicValue = metallic;
    float roughnessValue = clamp(roughness, 0.05, 1.0); // Prevent division by zero

    // Base reflectance (F0) - mix between dielectric (0.04) and metallic (albedo)
    vec3 F0 = mix(specularColor, albedo, metallicValue);

    // If specular texture is provided, use it to modulate F0 or metallic
    if (hasSpecularTexture != 0) {
        vec3 specularSample = texture(material_specular, TexCoord).rgb;
        // Assume specular texture contains metallic in R channel, roughness in G channel
        metallicValue *= specularSample.r;
        roughnessValue *= specularSample.g;
        F0 = mix(F0, albedo, metallicValue);
    }

    // Calculate lighting
    vec3 Lo = vec3(0.0);

    // Directional light
    Lo += calculatePBRDirectionalLight(directLight, norm, viewDirection, albedo, metallicValue, roughnessValue, F0);

    // Point lights
    for (int i = 0; i < numPointLights && i < MAX_LIGHTS; i++) {
        Lo += calculatePBRPointLight(pointLights[i], FragPos, norm, viewDirection, albedo, metallicValue, roughnessValue, F0);
    }

    // Ambient lighting (simple approximation)
    vec3 ambient = vec3(0.03) * albedo;
    vec3 color = ambient + Lo;

    // HDR tonemapping (simple Reinhard)
    color = color / (color + vec3(1.0));

    // Gamma correction
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, alpha);
}
