#version 330 core

#include "SharedConstants.h"

#define AMBIENT_INTENSITY 0.35
#define BACK_FACE_INTENSITY (AMBIENT_INTENSITY / 2.0)

#define LIGHT1_DIR vec3(1.0, 0.5, 0.5)
#define LIGHT1_INTENSITY 0.8

#define LIGHT2_DIR vec3(-1.0, 0.5, 0.5)
#define LIGHT2_INTENSITY 0.2

#define MAX_Z_FADE_DISTANCE 32.0

// Vertex attributes (inputs)
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in uint a_colour;
layout(location = 3) in vec2 a_texcoord;

// Uniform block (must match C++ VertexConstants struct with std140 layout)
layout(std140, binding = 0) uniform VertexConstants
{
    mat4 WVP;                       // 64 bytes (4x vec4)
    mat4 W;                         // 64 bytes (4x vec4)
    vec4 Palette[PALETTE_SIZE];     // 320 bytes (20x vec4)
    vec3 EyePos;                    // 12 bytes
    float z_fade;                   // 4 bytes
    float fog_density;              // 4 bytes
    uint fog_colour_idx;            // 4 bytes
    uint lighting;                  // 4 bytes
    uint padding;                   // 4 bytes (alignment)
};

// Outputs to fragment shader
out vec4 v_colour;
out vec2 v_texcoord;

void main()
{
    // Transform position to clip space
    // Note: Matrices are transposed when uploaded from DirectXMath (row-major) to GLSL (column-major)
    gl_Position = WVP * vec4(a_position, 1.0);
    v_texcoord = a_texcoord;

    float lightLevel = 1.0;

    if (lighting != 0u)
    {
        // Transform the model normal into a world direction
        vec3 transformedNormal = mat3(W) * a_normal;

        // Determine direction of the vertex from the eye position
        vec3 vertexDir = (W * vec4(a_position, 1.0)).xyz - EyePos;

        // If the front face is visible we'll use normal lighting
        if (dot(vertexDir, transformedNormal) < 0.0)
        {
            // Base ambient light level
            lightLevel = AMBIENT_INTENSITY;

            // Light 1 from back right
            float lightIntensity = dot(transformedNormal, normalize(LIGHT1_DIR));
            if (lightIntensity > 0.0)
                lightLevel += lightIntensity * LIGHT1_INTENSITY;

            // Light 2 from front-left
            lightIntensity = dot(transformedNormal, normalize(LIGHT2_DIR));
            if (lightIntensity > 0.0)
                lightLevel += lightIntensity * LIGHT2_INTENSITY;
        }
        else
        {
            // Back faces (underneath map) get a fraction of ambient
            lightLevel = BACK_FACE_INTENSITY;
        }
    }

    vec4 face_colour = clamp(lightLevel, 0.0, 1.0) * Palette[a_colour];
    float fog_level = 1.0 / exp(length(gl_Position.xyz) * fog_density);
    v_colour = mix(Palette[fog_colour_idx], face_colour, fog_level);

    if (z_fade > 0.0)
    {
        float z = (W * vec4(a_position, 1.0)).z;
        z = clamp(z, 0.0, MAX_Z_FADE_DISTANCE);

        float fade = 1.0 / exp(z * z_fade);
        v_colour *= fade;
    }
}
