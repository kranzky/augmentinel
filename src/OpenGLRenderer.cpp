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

    return true;
}

void OpenGLRenderer::BeginScene() {
    // Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Phase 1: Just clear screen
    // Phase 2+: Set up view/projection matrices, update uniforms
}

void OpenGLRenderer::Render(IGame* pGame) {
    // Phase 1: Empty - just clear screen
    // Phase 2+: Use shader program and render game objects
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
