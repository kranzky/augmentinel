#include "DebugOverlay.h"

// Simple overlay shaders for 2D textured quads
static const char* overlayVertexShader = R"(
#version 330 core
layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 a_texcoord;

out vec2 v_texcoord;

uniform vec2 u_position;
uniform vec2 u_size;

void main() {
    // Convert from screen coordinates to clip space (-1 to 1)
    vec2 pos = a_position * u_size + u_position;
    pos = pos * 2.0 - 1.0;  // [0,1] -> [-1,1]
    pos.y = -pos.y;  // Flip Y

    gl_Position = vec4(pos, 0.0, 1.0);
    v_texcoord = a_texcoord;
}
)";

static const char* overlayFragmentShader = R"(
#version 330 core
in vec2 v_texcoord;
out vec4 fragColor;

uniform sampler2D u_texture;

void main() {
    fragColor = texture(u_texture, v_texcoord);
}
)";

DebugOverlay::DebugOverlay() {
}

DebugOverlay::~DebugOverlay() {
    // Clean up textures
    for (auto& line : m_lines) {
        if (line.texture) {
            glDeleteTextures(1, &line.texture);
        }
    }

    if (m_overlayVBO) glDeleteBuffers(1, &m_overlayVBO);
    if (m_overlayVAO) glDeleteVertexArrays(1, &m_overlayVAO);
    if (m_overlayProgram) glDeleteProgram(m_overlayProgram);

    if (m_font) {
        TTF_CloseFont(m_font);
    }
    TTF_Quit();
}

bool DebugOverlay::Init(int windowWidth, int windowHeight) {
    m_windowWidth = windowWidth;
    m_windowHeight = windowHeight;

    // Initialize SDL_ttf
    if (TTF_Init() < 0) {
        SDL_Log("ERROR: TTF_Init failed: %s", TTF_GetError());
        return false;
    }

    // Try to load a monospace font (try multiple locations)
    const char* fontPaths[] = {
        "/System/Library/Fonts/Monaco.ttf",                    // macOS
        "/System/Library/Fonts/Courier New.ttf",               // macOS
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", // Linux
        "/usr/share/fonts/TTF/DejaVuSansMono.ttf",             // Linux
        "C:\\Windows\\Fonts\\consola.ttf"                      // Windows
    };

    for (const char* path : fontPaths) {
        m_font = TTF_OpenFont(path, 16);  // 16pt font
        if (m_font) {
            break;
        }
    }

    if (!m_font) {
        SDL_Log("ERROR: Could not load any debug font");
        return false;
    }

    // Compile overlay shaders
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &overlayVertexShader, nullptr);
    glCompileShader(vs);

    GLint success;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(vs, 512, nullptr, log);
        SDL_Log("ERROR: Overlay vertex shader compilation failed: %s", log);
        return false;
    }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &overlayFragmentShader, nullptr);
    glCompileShader(fs);

    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(fs, 512, nullptr, log);
        SDL_Log("ERROR: Overlay fragment shader compilation failed: %s", log);
        return false;
    }

    m_overlayProgram = glCreateProgram();
    glAttachShader(m_overlayProgram, vs);
    glAttachShader(m_overlayProgram, fs);
    glLinkProgram(m_overlayProgram);

    glGetProgramiv(m_overlayProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(m_overlayProgram, 512, nullptr, log);
        SDL_Log("ERROR: Overlay shader program linking failed: %s", log);
        return false;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    // Create VAO and VBO for textured quad
    glGenVertexArrays(1, &m_overlayVAO);
    glGenBuffers(1, &m_overlayVBO);

    glBindVertexArray(m_overlayVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_overlayVBO);

    // Quad vertices: position (xy) and texcoord (uv)
    float vertices[] = {
        // positions   // texcoords
        0.0f, 1.0f,    0.0f, 1.0f,  // bottom-left
        1.0f, 1.0f,    1.0f, 1.0f,  // bottom-right
        0.0f, 0.0f,    0.0f, 0.0f,  // top-left
        1.0f, 0.0f,    1.0f, 0.0f   // top-right
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);

    return true;
}

void DebugOverlay::SetText(const std::vector<std::string>& lines) {
    // Resize line array if needed
    while (m_lines.size() < lines.size()) {
        m_lines.push_back(TextLine());
    }

    // Update textures for each line
    for (size_t i = 0; i < lines.size(); i++) {
        if (i < m_lines.size() && m_lines[i].text != lines[i]) {
            UpdateTexture(m_lines[i], lines[i]);
        }
    }
}

void DebugOverlay::UpdateTexture(TextLine& line, const std::string& text) {
    if (!m_font) return;

    line.text = text;

    // Delete old texture if exists
    if (line.texture) {
        glDeleteTextures(1, &line.texture);
        line.texture = 0;
    }

    if (text.empty()) return;

    // Render text to surface
    SDL_Surface* textSurface = TTF_RenderText_Blended(m_font, text.c_str(), m_textColor);
    if (!textSurface) {
        SDL_Log("ERROR: TTF_RenderText failed: %s", TTF_GetError());
        return;
    }

    line.width = textSurface->w;
    line.height = textSurface->h;

    // Create OpenGL texture
    glGenTextures(1, &line.texture);
    glBindTexture(GL_TEXTURE_2D, line.texture);

    // Convert surface to RGBA if needed
    SDL_Surface* rgbaSurface = SDL_ConvertSurfaceFormat(textSurface, SDL_PIXELFORMAT_RGBA32, 0);
    if (rgbaSurface) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rgbaSurface->w, rgbaSurface->h, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, rgbaSurface->pixels);
        SDL_FreeSurface(rgbaSurface);
    }

    SDL_FreeSurface(textSurface);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void DebugOverlay::Render() {
    if (m_lines.empty() || !m_overlayProgram) {
        return;
    }

    // Save OpenGL state
    GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
    GLboolean cullFaceEnabled = glIsEnabled(GL_CULL_FACE);
    GLboolean blendEnabled = glIsEnabled(GL_BLEND);
    GLint blendSrc, blendDst;
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &blendSrc);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &blendDst);

    // Set up for overlay rendering
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);  // Disable culling for 2D overlay
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(m_overlayProgram);
    glBindVertexArray(m_overlayVAO);

    // Render each line
    float y = 10.0f;  // Start 10px from top
    float lineHeight = 20.0f;
    float padding = 4.0f;

    for (const auto& line : m_lines) {
        if (!line.texture || line.text.empty()) continue;

        float x = 10.0f;  // 10px from left
        RenderQuad(x / m_windowWidth, y / m_windowHeight,
                   (float)line.width / m_windowWidth, (float)line.height / m_windowHeight,
                   line.texture);

        y += lineHeight;
    }

    glBindVertexArray(0);
    glUseProgram(0);

    // Restore OpenGL state
    if (!depthTestEnabled) glDisable(GL_DEPTH_TEST);
    else glEnable(GL_DEPTH_TEST);

    if (!cullFaceEnabled) glDisable(GL_CULL_FACE);
    else glEnable(GL_CULL_FACE);

    if (!blendEnabled) glDisable(GL_BLEND);
    glBlendFunc(blendSrc, blendDst);
}

void DebugOverlay::RenderQuad(float x, float y, float width, float height, GLuint texture) {
    GLint posLoc = glGetUniformLocation(m_overlayProgram, "u_position");
    GLint sizeLoc = glGetUniformLocation(m_overlayProgram, "u_size");
    GLint texLoc = glGetUniformLocation(m_overlayProgram, "u_texture");

    glUniform2f(posLoc, x, y);
    glUniform2f(sizeLoc, width, height);
    glUniform1i(texLoc, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void DebugOverlay::OnResize(int windowWidth, int windowHeight) {
    m_windowWidth = windowWidth;
    m_windowHeight = windowHeight;
}
