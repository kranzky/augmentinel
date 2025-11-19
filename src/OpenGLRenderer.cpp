#include "Platform.h"
#include "OpenGLRenderer.h"

OpenGLRenderer::OpenGLRenderer(int width, int height)
    : m_width(width), m_height(height) {
}

OpenGLRenderer::~OpenGLRenderer() {
    // Cleanup OpenGL resources
    if (m_vao) {
        glDeleteVertexArrays(1, &m_vao);
    }
    if (m_testVBO) {
        glDeleteBuffers(1, &m_testVBO);
    }
    if (m_sentinelProgram) {
        glDeleteProgram(m_sentinelProgram);
    }
    if (m_effectProgram) {
        glDeleteProgram(m_effectProgram);
    }
    if (m_vertexConstantsUBO) {
        glDeleteBuffers(1, &m_vertexConstantsUBO);
    }
    if (m_pixelConstantsUBO) {
        glDeleteBuffers(1, &m_pixelConstantsUBO);
    }
}

bool OpenGLRenderer::Init() {
    // Set up basic OpenGL state
    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);  // Dark blue background for testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    SDL_Log("OpenGLRenderer: Initializing shader pipeline...");

    // Load and compile Sentinel shaders
    std::string sentinelVertSrc = LoadShaderFile("Sentinel.vert");
    std::string sentinelFragSrc = LoadShaderFile("Sentinel.frag");

    if (sentinelVertSrc.empty() || sentinelFragSrc.empty()) {
        SDL_Log("ERROR: Failed to load Sentinel shader files");
        return false;
    }

    GLuint sentinelVS = CompileShader(sentinelVertSrc.c_str(), GL_VERTEX_SHADER, "Sentinel.vert");
    GLuint sentinelFS = CompileShader(sentinelFragSrc.c_str(), GL_FRAGMENT_SHADER, "Sentinel.frag");

    if (sentinelVS == 0 || sentinelFS == 0) {
        SDL_Log("ERROR: Failed to compile Sentinel shaders");
        return false;
    }

    m_sentinelProgram = LinkProgram(sentinelVS, sentinelFS, "Sentinel");

    if (m_sentinelProgram == 0) {
        SDL_Log("ERROR: Failed to link Sentinel shader program");
        return false;
    }

    // Load and compile Effect shaders
    std::string effectVertSrc = LoadShaderFile("Effect.vert");
    std::string effectFragSrc = LoadShaderFile("Effect.frag");

    if (effectVertSrc.empty() || effectFragSrc.empty()) {
        SDL_Log("ERROR: Failed to load Effect shader files");
        return false;
    }

    GLuint effectVS = CompileShader(effectVertSrc.c_str(), GL_VERTEX_SHADER, "Effect.vert");
    GLuint effectFS = CompileShader(effectFragSrc.c_str(), GL_FRAGMENT_SHADER, "Effect.frag");

    if (effectVS == 0 || effectFS == 0) {
        SDL_Log("ERROR: Failed to compile Effect shaders");
        return false;
    }

    m_effectProgram = LinkProgram(effectVS, effectFS, "Effect");

    if (m_effectProgram == 0) {
        SDL_Log("ERROR: Failed to link Effect shader program");
        return false;
    }

    SDL_Log("OpenGLRenderer: Shader pipeline initialized successfully");
    SDL_Log("  - Sentinel program: %u", m_sentinelProgram);
    SDL_Log("  - Effect program: %u", m_effectProgram);

    // Create uniform buffers (UBOs)
    SDL_Log("OpenGLRenderer: Creating uniform buffers...");

    // Create vertex constants UBO
    glGenBuffers(1, &m_vertexConstantsUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, m_vertexConstantsUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(VertexConstants), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_vertexConstantsUBO);  // Binding point 0
    SDL_Log("  - Vertex constants UBO: %u (size: %zu bytes)", m_vertexConstantsUBO, sizeof(VertexConstants));

    // Create pixel constants UBO
    glGenBuffers(1, &m_pixelConstantsUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, m_pixelConstantsUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(PixelConstants), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_pixelConstantsUBO);  // Binding point 1
    SDL_Log("  - Pixel constants UBO: %u (size: %zu bytes)", m_pixelConstantsUBO, sizeof(PixelConstants));

    // Unbind buffer
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Set uniform block bindings for shader programs
    GLuint vertexBlockIndex = glGetUniformBlockIndex(m_sentinelProgram, "VertexConstants");
    if (vertexBlockIndex != GL_INVALID_INDEX) {
        glUniformBlockBinding(m_sentinelProgram, vertexBlockIndex, 0);
        SDL_Log("  - Bound VertexConstants in Sentinel program to binding point 0");
    } else {
        SDL_Log("WARNING: VertexConstants block not found in Sentinel program");
    }

    GLuint pixelBlockIndexSentinel = glGetUniformBlockIndex(m_sentinelProgram, "PixelConstants");
    if (pixelBlockIndexSentinel != GL_INVALID_INDEX) {
        glUniformBlockBinding(m_sentinelProgram, pixelBlockIndexSentinel, 1);
        SDL_Log("  - Bound PixelConstants in Sentinel program to binding point 1");
    } else {
        SDL_Log("WARNING: PixelConstants block not found in Sentinel program");
    }

    GLuint pixelBlockIndexEffect = glGetUniformBlockIndex(m_effectProgram, "PixelConstants");
    if (pixelBlockIndexEffect != GL_INVALID_INDEX) {
        glUniformBlockBinding(m_effectProgram, pixelBlockIndexEffect, 1);
        SDL_Log("  - Bound PixelConstants in Effect program to binding point 1");
    } else {
        SDL_Log("WARNING: PixelConstants block not found in Effect program");
    }

    // Check for OpenGL errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        SDL_Log("ERROR: OpenGL error during UBO creation: 0x%x", err);
        return false;
    }

    SDL_Log("OpenGLRenderer: Uniform buffers created successfully");

    // Create test triangle (Phase 2.9)
    SDL_Log("OpenGLRenderer: Creating test triangle...");

    // Define test triangle vertices (RGB triangle for testing)
    Vertex testVerts[3] = {
        Vertex(-0.5f, -0.5f, 0.0f, 0),  // Bottom-left (will use palette[0] - red)
        Vertex( 0.5f, -0.5f, 0.0f, 1),  // Bottom-right (will use palette[1] - green)
        Vertex( 0.0f,  0.5f, 0.0f, 2),  // Top (will use palette[2] - blue)
    };

    // Create and upload vertex buffer
    glGenBuffers(1, &m_testVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_testVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(testVerts), testVerts, GL_STATIC_DRAW);
    SDL_Log("  - Test VBO created: %u (%zu bytes)", m_testVBO, sizeof(testVerts));

    // Create VAO and configure vertex attributes
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    // Attribute 0: position (vec3)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));

    // Attribute 1: normal (vec3)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    // Attribute 2: color (uint)
    glEnableVertexAttribArray(2);
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(Vertex), (void*)offsetof(Vertex, colour));

    // Attribute 3: texcoord (vec2)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texcoord));

    // Unbind VAO
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    SDL_Log("  - VAO created: %u", m_vao);

    // Check for errors
    err = glGetError();
    if (err != GL_NO_ERROR) {
        SDL_Log("ERROR: OpenGL error during test triangle creation: 0x%x", err);
        return false;
    }

    SDL_Log("OpenGLRenderer: Test triangle created successfully");

    // Phase 2.11: Initialize camera to a good starting position
    // Position camera back from origin so we can see the test triangle at (0, 0, 5)
    m_camera.SetPosition(XMFLOAT3(0.0f, 0.0f, -5.0f));  // 5 units back
    m_camera.SetRotation(XMFLOAT3(0.0f, 0.0f, 0.0f));   // Looking forward (+Z)

    SDL_Log("OpenGLRenderer: Camera initialized at (0, 0, -5)");

    return true;
}

void OpenGLRenderer::BeginScene() {
    // Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Phase 2.11: Set up view and projection matrices
    XMMATRIX view = m_camera.GetViewMatrix();
    float aspectRatio = static_cast<float>(m_width) / static_cast<float>(m_height);
    XMMATRIX proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, aspectRatio, NEAR_CLIP, FAR_CLIP);
    m_mViewProjection = view * proj;

    // Store camera position in vertex constants for lighting calculations
    XMFLOAT3 eyePos = m_camera.GetPosition();
    m_vertexConstants.EyePos = eyePos;

    // Update pixel constants (view effects)
    // Vertex constants will be updated in Render() after WVP is set
    UpdatePixelConstants();
}

void OpenGLRenderer::Render(IGame* pGame) {
    // Phase 2.10-2.11: Render test triangle with 3D projection

    // Phase 2.11: Create world matrix - position triangle 5 units in front of camera
    XMMATRIX world = XMMatrixTranslation(0.0f, 0.0f, 5.0f);

    // Calculate final WVP matrix (World * View * Projection)
    // IMPORTANT: Transpose for GLSL (DirectXMath is row-major, GLSL is column-major)
    m_vertexConstants.WVP = XMMatrixTranspose(world * m_mViewProjection);
    m_vertexConstants.W = XMMatrixTranspose(world);

    // Set palette colors (RGB for vertices with palette indices 0, 1, 2)
    m_vertexConstants.Palette[0] = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);  // Red
    m_vertexConstants.Palette[1] = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);  // Green
    m_vertexConstants.Palette[2] = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);  // Blue

    // Update vertex constants UBO with new values
    UpdateVertexConstants();

    // Bind shader program
    glUseProgram(m_sentinelProgram);
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        SDL_Log("ERROR: Failed to bind shader program: 0x%x", err);
        return;
    }

    // Bind VAO
    glBindVertexArray(m_vao);
    err = glGetError();
    if (err != GL_NO_ERROR) {
        SDL_Log("ERROR: Failed to bind VAO: 0x%x", err);
        return;
    }

    // Draw triangle
    glDrawArrays(GL_TRIANGLES, 0, 3);
    err = glGetError();
    if (err != GL_NO_ERROR) {
        SDL_Log("ERROR: Failed to draw triangle: 0x%x", err);
        return;
    }

    // Unbind
    glBindVertexArray(0);
    glUseProgram(0);

    // Phase 3+: Render game objects
    // pGame->Render(this);
}

void OpenGLRenderer::EndScene() {
    // Nothing needed here - SDL_GL_SwapWindow is called by Application
}

void OpenGLRenderer::DrawModel(Model& model, const Model& linkedModel) {
    // Phase 3: Implement model rendering
    // For now, stub
}

void OpenGLRenderer::DrawControllers() {
    // VR only - no-op in flat mode
}

bool OpenGLRenderer::IsPointerVisible() const {
    // Phase 3+: Implement pointer visibility check
    return false;
}

void OpenGLRenderer::OnResize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
    glViewport(0, 0, width, height);
    SDL_Log("Viewport resized to %dx%d", width, height);
}

void OpenGLRenderer::MouseMove(int xrel, int yrel) {
    // Phase 4+: Implement mouse look
    // For now, just call base class
    View::MouseMove(xrel, yrel);
}

// Pure virtual implementations - return camera data
XMVECTOR OpenGLRenderer::GetEyePositionVector() const {
    return m_camera.GetPositionVector();
}

XMVECTOR OpenGLRenderer::GetViewPositionVector() const {
    return m_camera.GetPositionVector();
}

XMVECTOR OpenGLRenderer::GetViewDirectionVector() const {
    return m_camera.GetDirectionVector();
}

XMVECTOR OpenGLRenderer::GetViewUpVector() const {
    return m_camera.GetUpVector();
}

XMMATRIX OpenGLRenderer::GetViewProjectionMatrix() const {
    return m_mViewProjection;
}

XMMATRIX OpenGLRenderer::GetOrthographicMatrix() const {
    // Return orthographic matrix for UI rendering
    float width = static_cast<float>(m_width);
    float height = static_cast<float>(m_height);
    return XMMatrixOrthographicLH(width, height, 0.0f, 1.0f);
}

void OpenGLRenderer::GetSelectionRay(XMVECTOR& vPos, XMVECTOR& vDir) const {
    vPos = m_camera.GetPositionVector();
    vDir = m_camera.GetDirectionVector();
}

// Shader loading helpers

std::string OpenGLRenderer::LoadShaderFile(const std::string& filename) {
    // Try to open the shader file from the shaders/ directory
    std::string path = "shaders/" + filename;
    std::ifstream file(path);

    if (!file.is_open()) {
        SDL_Log("ERROR: Failed to open shader file: %s", path.c_str());
        return "";
    }

    // Read entire file into string
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    SDL_Log("Loaded shader file: %s (%zu bytes)", path.c_str(), source.length());
    return source;
}

GLuint OpenGLRenderer::CompileShader(const char* source, GLenum type, const char* name) {
    // Create shader object
    GLuint shader = glCreateShader(type);
    if (shader == 0) {
        SDL_Log("ERROR: Failed to create shader object for %s", name);
        return 0;
    }

    // Set shader source and compile
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    // Check compilation status
    GLint status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE) {
        // Get error log
        GLint logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

        if (logLength > 0) {
            std::vector<char> log(logLength);
            glGetShaderInfoLog(shader, logLength, nullptr, log.data());
            SDL_Log("ERROR: Shader compilation failed for %s:\n%s", name, log.data());
        } else {
            SDL_Log("ERROR: Shader compilation failed for %s (no log available)", name);
        }

        glDeleteShader(shader);
        return 0;
    }

    SDL_Log("Compiled shader: %s", name);
    return shader;
}

GLuint OpenGLRenderer::LinkProgram(GLuint vs, GLuint fs, const char* name) {
    // Create program object
    GLuint program = glCreateProgram();
    if (program == 0) {
        SDL_Log("ERROR: Failed to create program object for %s", name);
        return 0;
    }

    // Attach shaders
    glAttachShader(program, vs);
    glAttachShader(program, fs);

    // Link program
    glLinkProgram(program);

    // Check link status
    GLint status = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &status);

    if (status == GL_FALSE) {
        // Get error log
        GLint logLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

        if (logLength > 0) {
            std::vector<char> log(logLength);
            glGetProgramInfoLog(program, logLength, nullptr, log.data());
            SDL_Log("ERROR: Program linking failed for %s:\n%s", name, log.data());
        } else {
            SDL_Log("ERROR: Program linking failed for %s (no log available)", name);
        }

        glDeleteProgram(program);
        return 0;
    }

    // Detach and delete shader objects (no longer needed after linking)
    glDetachShader(program, vs);
    glDetachShader(program, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);

    SDL_Log("Linked shader program: %s", name);
    return program;
}

// Uniform buffer update methods

void OpenGLRenderer::UpdateVertexConstants() {
    glBindBuffer(GL_UNIFORM_BUFFER, m_vertexConstantsUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(VertexConstants), &m_vertexConstants);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void OpenGLRenderer::UpdatePixelConstants() {
    glBindBuffer(GL_UNIFORM_BUFFER, m_pixelConstantsUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PixelConstants), &m_pixelConstants);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
