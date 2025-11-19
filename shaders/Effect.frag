#version 330 core

// Scene texture to sample from
uniform sampler2D u_sceneTexture;

// Uniform block for pixel constants (must match C++ PixelConstants struct)
layout(std140, binding = 1) uniform PixelConstants
{
    float dissolved;        // 0.0 = fully visible, 1.0 = fully dissolved (for models)
    float noise;            // 0.0 <= noise < 1.0 (animated noise offset)
    float view_dissolve;    // 0.0 = normal, 1.0 = dissolved (view effect)
    float view_desaturate;  // 0.0 = colour, 1.0 = greyscale (view effect)
    float view_fade;        // 0.0 = normal, 1.0 = black (view effect)
    float padding[3];
};

// Input from vertex shader
in vec2 v_texcoord;

// Output
out vec4 FragColor;

// Random noise function
// https://stackoverflow.com/questions/5149544/can-i-generate-a-random-number-inside-a-pixel-shader#answer-10625698
float rnd(vec2 uv)
{
    return fract(cos(mod(123456780.0, 1024.0 * dot(uv, vec2(23.14069263277926, 2.6651441426902251)))));
}

void main()
{
    // View dissolve effect - discard pixels based on noise
    if (view_dissolve > 0.0)
    {
        // Discard pixel if random value determines it shouldn't be visible
        vec2 uv = fract(v_texcoord + vec2(noise, noise));
        float random_value = rnd(uv);

        // HLSL clip(x) discards if x < 0
        if (random_value - view_dissolve < 0.0)
            discard;
    }

    // Sample the scene texture
    vec4 colour = texture(u_sceneTexture, v_texcoord);

    // View desaturate effect - convert to grayscale
    if (view_desaturate > 0.0)
    {
        // Calculate luminance using standard coefficients
        float lum = dot(colour.rgb, vec3(0.299, 0.587, 0.114));

        // Mix between colour and grayscale based on desaturate amount
        colour.rgb = mix(colour.rgb, vec3(lum, lum, lum), view_desaturate);
    }

    // View fade effect - fade to black
    if (view_fade > 0.0)
    {
        colour.rgb *= (1.0 - view_fade);
    }

    FragColor = colour;
}
