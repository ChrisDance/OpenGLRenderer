
// particle_fragment.glsl
#version 330 core
in vec2 TexCoord;
in vec4 ParticleColor;

out vec4 FragColor;

uniform sampler2D particleTexture;
uniform bool useTexture = false;

void main() {
    vec4 texColor = vec4(1.0);

    if (useTexture) {
        texColor = texture(particleTexture, TexCoord);
    } else {
        // Create a simple circular gradient for particles without texture
        vec2 centeredCoord = TexCoord - 0.5;
        float dist = length(centeredCoord);
        float alpha = 1.0 - smoothstep(0.0, 0.5, dist);
        texColor = vec4(1.0, 1.0, 1.0, alpha);
    }

    FragColor = ParticleColor * texColor;

    // Discard fully transparent pixels to improve performance
    if (FragColor.a < 0.01) {
        discard;
    }
}
