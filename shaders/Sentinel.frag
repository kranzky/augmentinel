#version 330 core

// Uniform block for pixel constants
layout(std140, binding = 1) uniform PixelConstants
{
    float dissolved;    // 0.0 = fully visible, 1.0 = fully dissolved
    float noise;        // 0.0 <= noise < 1.0
    float view_dissolve;
    float view_desaturate;
    float view_fade;
    float padding[3];
};

// Inputs from vertex shader
in vec4 v_colour;
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
    if (dissolved == 0.0)
    {
        FragColor = v_colour;
        return;
    }

    vec2 uv = fract(v_texcoord + vec2(noise, noise));
    float random_value = rnd(uv);

    // HLSL clip(x) discards if x < 0, so clip(random - dissolved) means discard if random < dissolved
    if (random_value - dissolved < 0.0)
        discard;

    FragColor = v_colour;
}
