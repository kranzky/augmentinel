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

    // Cleanup framebuffer objects (Phase 4.5)
    if (m_sceneFBO) {
        glDeleteFramebuffers(1, &m_sceneFBO);
    }
    if (m_sceneTexture) {
        glDeleteTextures(1, &m_sceneTexture);
    }
    if (m_sceneDepthRBO) {
        glDeleteRenderbuffers(1, &m_sceneDepthRBO);
    }

    // Cleanup model buffers
    for (auto& pair : m_modelVBOs) {
        glDeleteBuffers(1, &pair.second);
    }
    for (auto& pair : m_modelIBOs) {
        glDeleteBuffers(1, &pair.second);
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

    // Create VAO (vertex attributes will be set up per-draw in DrawModel)
    glGenVertexArrays(1, &m_vao);
    SDL_Log("  - VAO created: %u", m_vao);

    // Check for errors
    err = glGetError();
    if (err != GL_NO_ERROR) {
        SDL_Log("ERROR: OpenGL error during VAO creation: 0x%x", err);
        return false;
    }

    // Initialize camera (game will set actual position)
    m_camera.SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
    m_camera.SetRotation(XMFLOAT3(0.0f, 0.0f, 0.0f));

    SDL_Log("OpenGLRenderer: Camera initialized");

    // Initialize framebuffers for post-processing (Phase 4.5)
    InitFramebuffers();

    return true;
}

void OpenGLRenderer::BeginScene() {
    // Reset performance stats
    m_drawCallCount = 0;

    // Phase 4.5: Conditional rendering path
    // If pixel shader effects are active, render to FBO for post-processing
    // Otherwise, render directly to screen for better performance
    if (PixelShaderEffectsActive()) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_sceneFBO);
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Enable backface culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);  // Changed from GL_CCW - models use clockwise winding

    // Set clear color based on fill_colour_idx (palette-based background)
    if (m_fill_colour_idx >= 0 && m_fill_colour_idx < 16) {
        XMFLOAT4 clearColor = m_vertexConstants.Palette[m_fill_colour_idx];
        glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
    } else {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  // Default black
    }

    // Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up view and projection matrices
    XMMATRIX view = m_camera.GetViewMatrix();
    float aspectRatio = static_cast<float>(m_width) / static_cast<float>(m_height);
    float fovRadians = XMConvertToRadians(m_verticalFOV);
    XMMATRIX proj = XMMatrixPerspectiveFovLH(fovRadians, aspectRatio, NEAR_CLIP, FAR_CLIP);
    m_mViewProjection = view * proj;

    // Store camera position in vertex constants for lighting calculations
    XMFLOAT3 eyePos = m_camera.GetPosition();
    m_vertexConstants.EyePos = eyePos;

    // Update pixel constants (view effects)
    UpdatePixelConstants();
}

void OpenGLRenderer::Render(IGame* pGame) {
    // Bind sentinel shader program
    glUseProgram(m_sentinelProgram);

    // Bind VAO
    glBindVertexArray(m_vao);

    // Call game to render its models
    // Game will call our DrawModel() for each model it wants to render
    if (pGame) {
        pGame->Render(this);
    }

    // Unbind VAO
    glBindVertexArray(0);
}

void OpenGLRenderer::EndScene() {
    // Phase 4.5: Post-processing with Effect shader
    // If no effects are active, we rendered directly to screen - nothing to do
    if (!PixelShaderEffectsActive()) {
        return;
    }

    // Bind default framebuffer (render to screen)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Disable depth test for 2D post-processing
    glDisable(GL_DEPTH_TEST);

    // Bind Effect shader program
    glUseProgram(m_effectProgram);

    // Bind VAO (required even though we generate quad from gl_VertexID)
    glBindVertexArray(m_vao);

    // Bind scene texture to texture unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_sceneTexture);

    // Set the texture uniform (u_sceneTexture) to texture unit 0
    GLint texLocation = glGetUniformLocation(m_effectProgram, "u_sceneTexture");
    if (texLocation != -1) {
        glUniform1i(texLocation, 0);
    }

    // Pixel constants are already updated and bound to UBO binding point 1
    // Effect shader uses them for view_dissolve, view_desaturate, view_fade

    // Draw fullscreen quad (Effect vertex shader generates it from gl_VertexID)
    // 4 vertices for TRIANGLE_STRIP: (0,0), (1,0), (0,1), (1,1) in UV space
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Unbind VAO and texture
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Re-enable depth test
    glEnable(GL_DEPTH_TEST);

    // Check for OpenGL errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        SDL_Log("ERROR: EndScene post-processing failed with GL error: 0x%x", err);
    }
}

void OpenGLRenderer::DrawModel(Model& model, const Model& linkedModel) {
    // Check if model is valid
    if (!model) {
        return;
    }

    // Upload if not already uploaded (using vertex buffer pointer as cache key)
    const void* cacheKey = model.m_pVertices.get();
    if (!m_modelVBOs.count(cacheKey)) {
        UploadModel(model);
    }

    // Calculate world matrix
    auto world = model.GetWorldMatrix(linkedModel);

    // Calculate WVP (world-view-projection)
    // Use orthographic projection for UI elements (energy icons)
    auto projection = model.orthographic ? GetOrthographicMatrix() : m_mViewProjection;
    auto wvp = world * projection;

    // NOTE: DirectXMath matrices work directly with GLSL without transposition
    // Even though DirectXMath uses row-major and GLSL uses column-major,
    // the memory layout is compatible as-is.
    m_vertexConstants.WVP = wvp;
    m_vertexConstants.W = world;

    // Set eye position for lighting
    auto eyePos = m_camera.GetPosition();
    m_vertexConstants.EyePos = eyePos;

    // Set lighting flag
    m_vertexConstants.lighting = model.lighting ? 1 : 0;

    // Set dissolved value
    m_pixelConstants.dissolved = model.dissolved;

    // Update uniform buffers
    UpdateVertexConstants();
    UpdatePixelConstants();

    // Bind buffers (using vertex buffer pointer as cache key)
    glBindBuffer(GL_ARRAY_BUFFER, m_modelVBOs.at(cacheKey));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_modelIBOs.at(cacheKey));

    // Set up vertex attributes (same layout as test triangle)
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

    // Draw
    size_t indexCount = m_modelIndexCounts.at(cacheKey);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    m_drawCallCount++;

    // Check for OpenGL errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        SDL_Log("ERROR: DrawModel failed with GL error: 0x%x", err);
    }
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

    // Phase 4.5: Resize framebuffers to match new window size
    ResizeFramebuffers();
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
    // Uses same depth range as perspective projection for consistency
    float width = static_cast<float>(m_width);
    float height = static_cast<float>(m_height);
    return XMMatrixOrthographicLH(width, height, NEAR_CLIP, FAR_CLIP);
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

void OpenGLRenderer::UploadModel(const Model& model) {
    // Use vertex buffer pointer as cache key (identifies unique geometry)
    const void* cacheKey = model.m_pVertices.get();

    // Check if already uploaded
    if (m_modelVBOs.count(cacheKey)) {
        // Already uploaded, skip
        return;
    }

    // Get vertex and index data from model
    auto& vertices = *model.m_pVertices;
    auto& indices = *model.m_pIndices;

    // Create VBO
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
                 vertices.data(), GL_STATIC_DRAW);
    m_modelVBOs[cacheKey] = vbo;

    // Create IBO
    GLuint ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t),
                 indices.data(), GL_STATIC_DRAW);
    m_modelIBOs[cacheKey] = ibo;
    m_modelIndexCounts[cacheKey] = indices.size();

    // Unbind buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Check for OpenGL errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        SDL_Log("ERROR: UploadModel failed with GL error: 0x%x", err);
    }
}

void OpenGLRenderer::SetVerticalFOV(float fov) {
    m_verticalFOV = fov;
    SDL_Log("SetVerticalFOV: %.2f degrees", fov);
}

// Framebuffer management (Phase 4.5)

void OpenGLRenderer::InitFramebuffers() {
    SDL_Log("OpenGLRenderer: Initializing framebuffers for post-processing...");

    // Generate framebuffer
    glGenFramebuffers(1, &m_sceneFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_sceneFBO);

    // Create scene texture (color attachment)
    glGenTextures(1, &m_sceneTexture);
    glBindTexture(GL_TEXTURE_2D, m_sceneTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Attach texture to FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_sceneTexture, 0);

    // Create depth/stencil renderbuffer
    glGenRenderbuffers(1, &m_sceneDepthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_sceneDepthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Attach renderbuffer to FBO
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_sceneDepthRBO);

    // Check FBO completeness
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        SDL_Log("ERROR: Framebuffer is not complete! Status: 0x%x", status);
    } else {
        SDL_Log("  - Scene FBO: %u (%dx%d)", m_sceneFBO, m_width, m_height);
        SDL_Log("  - Scene texture: %u", m_sceneTexture);
        SDL_Log("  - Scene depth RBO: %u", m_sceneDepthRBO);
    }

    // Unbind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Check for OpenGL errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        SDL_Log("ERROR: OpenGL error during framebuffer creation: 0x%x", err);
    } else {
        SDL_Log("OpenGLRenderer: Framebuffers initialized successfully");
    }
}

void OpenGLRenderer::ResizeFramebuffers() {
    SDL_Log("OpenGLRenderer: Resizing framebuffers to %dx%d...", m_width, m_height);

    // Delete old texture and renderbuffer
    if (m_sceneTexture) {
        glDeleteTextures(1, &m_sceneTexture);
        m_sceneTexture = 0;
    }
    if (m_sceneDepthRBO) {
        glDeleteRenderbuffers(1, &m_sceneDepthRBO);
        m_sceneDepthRBO = 0;
    }

    // Bind FBO
    glBindFramebuffer(GL_FRAMEBUFFER, m_sceneFBO);

    // Recreate scene texture
    glGenTextures(1, &m_sceneTexture);
    glBindTexture(GL_TEXTURE_2D, m_sceneTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Re-attach texture to FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_sceneTexture, 0);

    // Recreate depth/stencil renderbuffer
    glGenRenderbuffers(1, &m_sceneDepthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_sceneDepthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Re-attach renderbuffer to FBO
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_sceneDepthRBO);

    // Check FBO completeness
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        SDL_Log("ERROR: Framebuffer is not complete after resize! Status: 0x%x", status);
    } else {
        SDL_Log("  - Framebuffers resized successfully");
    }

    // Unbind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Check for OpenGL errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        SDL_Log("ERROR: OpenGL error during framebuffer resize: 0x%x", err);
    }
}
