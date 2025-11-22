#version 330 core

// No vertex attributes - we generate a fullscreen quad from vertex ID
// Outputs to fragment shader
out vec2 v_texcoord;

void main()
{
    // Generate texture coords from vertex id (0, 1, 2, 3)
    // Vertex 0: (0, 0), Vertex 1: (1, 0), Vertex 2: (0, 1), Vertex 3: (1, 1)
    float u = float(gl_VertexID / 2);
    float v = float(gl_VertexID % 2);

    // Generate screen space coords from texture coords
    // Map [0, 1] to [-1, 1] for clip space
    float x = u * 2.0 - 1.0;
    float y = v * 2.0 - 1.0;

    gl_Position = vec4(x, y, 0.0, 1.0);

    // OpenGL framebuffers don't need Y-flip (unlike D3D)
    v_texcoord = vec2(u, v);
}
