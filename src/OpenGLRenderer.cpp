#include "Platform.h"
#include "OpenGLRenderer.h"
#include "Settings.h"

// Uniform block binding points, matching the shaders.
constexpr GLuint VERTEX_CONSTANTS_BINDING = 0;
constexpr GLuint PIXEL_CONSTANTS_BINDING = 1;

// The orthographic UI space is 1000 units high, with the origin at the bottom-left.
constexpr float ORTHO_HEIGHT = 1000.0f;

#ifndef NDEBUG
static void CheckGLError(const char* operation)
{
    for (auto err = glGetError(); err != GL_NO_ERROR; err = glGetError())
        SDL_Log("OpenGL error 0x%x in %s", err, operation);
}
#else
static void CheckGLError(const char*) {}
#endif

static std::string LoadShaderSource(const std::string& filename)
{
    std::ifstream file(g_resourcePath / "shaders" / filename, std::ios::binary);
    if (!file)
    {
        SDL_Log("Failed to open shader %s", filename.c_str());
        return {};
    }
    return { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
}

static GLuint CompileShader(const std::string& filename, GLenum type)
{
    auto source = LoadShaderSource(filename);
    if (source.empty())
        return 0;

    auto shader = glCreateShader(type);
    auto source_ptr = source.c_str();
    glShaderSource(shader, 1, &source_ptr, nullptr);
    glCompileShader(shader);

    GLint status = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        GLint length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(std::max(length, 1));
        glGetShaderInfoLog(shader, static_cast<GLsizei>(log.size()), nullptr, log.data());
        SDL_Log("Failed to compile %s:\n%s", filename.c_str(), log.data());
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

static GLuint LinkProgram(const std::string& name)
{
    auto vs = CompileShader(name + ".vert", GL_VERTEX_SHADER);
    auto fs = CompileShader(name + ".frag", GL_FRAGMENT_SHADER);

    GLuint program = 0;
    if (vs && fs)
    {
        program = glCreateProgram();
        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glLinkProgram(program);
        glDetachShader(program, vs);
        glDetachShader(program, fs);

        GLint status = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &status);
        if (status != GL_TRUE)
        {
            GLint length = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
            std::vector<char> log(std::max(length, 1));
            glGetProgramInfoLog(program, static_cast<GLsizei>(log.size()), nullptr, log.data());
            SDL_Log("Failed to link %s shaders:\n%s", name.c_str(), log.data());
            glDeleteProgram(program);
            program = 0;
        }
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

static void BindUniformBlock(GLuint program, const char* name, GLuint binding)
{
    auto index = glGetUniformBlockIndex(program, name);
    if (index != GL_INVALID_INDEX)
        glUniformBlockBinding(program, index, binding);
}

static GLuint CreateUniformBuffer(GLsizeiptr size, GLuint binding)
{
    GLuint ubo = 0;
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, binding, ubo);
    return ubo;
}

OpenGLRenderer::~OpenGLRenderer()
{
    for (auto& [key, mesh] : m_meshes)
        Destroy(mesh);

    DestroyRenderTargets();
    glDeleteBuffers(1, &m_vertexConstantsUBO);
    glDeleteBuffers(1, &m_pixelConstantsUBO);
    glDeleteVertexArrays(1, &m_effectVao);
    glDeleteProgram(m_sentinelProgram);
    glDeleteProgram(m_effectProgram);
}

bool OpenGLRenderer::Init(int width, int height)
{
    m_sentinelProgram = LinkProgram("Sentinel");
    m_effectProgram = LinkProgram("Effect");
    if (!m_sentinelProgram || !m_effectProgram)
        return false;

    BindUniformBlock(m_sentinelProgram, "VertexConstants", VERTEX_CONSTANTS_BINDING);
    BindUniformBlock(m_sentinelProgram, "PixelConstants", PIXEL_CONSTANTS_BINDING);
    BindUniformBlock(m_effectProgram, "PixelConstants", PIXEL_CONSTANTS_BINDING);

    glUseProgram(m_effectProgram);
    glUniform1i(glGetUniformLocation(m_effectProgram, "u_sceneTexture"), 0);
    glUseProgram(0);

    m_vertexConstantsUBO = CreateUniformBuffer(sizeof(VertexConstants), VERTEX_CONSTANTS_BINDING);
    m_pixelConstantsUBO = CreateUniformBuffer(sizeof(PixelConstants), PIXEL_CONSTANTS_BINDING);

    // The effect pass generates its full-screen quad from gl_VertexID, but the core
    // profile still requires a vertex array object to be bound.
    glGenVertexArrays(1, &m_effectVao);

    // Models use clockwise winding.
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
    glDepthFunc(GL_LESS);

    m_invert_mouse = GetFlag(INVERT_MOUSE_KEY, DEFAULT_INVERT_MOUSE);

    GLint max_samples = 1;
    glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
    m_samples = std::clamp(GetSetting(MSAA_SAMPLES_KEY, DEFAULT_MSAA_SAMPLES), 1, std::max(max_samples, 1));

    m_width = width;
    m_height = height;
    if (!CreateRenderTargets())
        return false;

    CheckGLError("OpenGLRenderer::Init");
    return true;
}

bool OpenGLRenderer::CreateRenderTargets()
{
    // A sample count of 0 gives ordinary single-sampled storage.
    auto samples = (m_samples > 1) ? m_samples : 0;

    glGenRenderbuffers(1, &m_sceneColourRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_sceneColourRBO);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_RGBA8, m_width, m_height);

    glGenRenderbuffers(1, &m_sceneDepthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_sceneDepthRBO);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH_COMPONENT24, m_width, m_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glGenFramebuffers(1, &m_sceneFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_sceneFBO);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_sceneColourRBO);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_sceneDepthRBO);
    auto scene_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    // Single-sampled texture that the scene is resolved into for the effect pass.
    glGenTextures(1, &m_resolveTexture);
    glBindTexture(GL_TEXTURE_2D, m_resolveTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &m_resolveFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_resolveFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_resolveTexture, 0);
    auto resolve_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (scene_status != GL_FRAMEBUFFER_COMPLETE || resolve_status != GL_FRAMEBUFFER_COMPLETE)
    {
        SDL_Log("Incomplete framebuffer (scene 0x%x, resolve 0x%x) at %dx%d with %d samples",
            scene_status, resolve_status, m_width, m_height, m_samples);
        return false;
    }

    return true;
}

void OpenGLRenderer::DestroyRenderTargets()
{
    glDeleteFramebuffers(1, &m_sceneFBO);
    glDeleteFramebuffers(1, &m_resolveFBO);
    glDeleteRenderbuffers(1, &m_sceneColourRBO);
    glDeleteRenderbuffers(1, &m_sceneDepthRBO);
    glDeleteTextures(1, &m_resolveTexture);
    m_sceneFBO = m_resolveFBO = m_sceneColourRBO = m_sceneDepthRBO = m_resolveTexture = 0;
}

void OpenGLRenderer::OnResize(uint32_t width, uint32_t height)
{
    // Minimised windows report a zero size; keep the old targets until restored.
    if (!width || !height || (static_cast<int>(width) == m_width && static_cast<int>(height) == m_height))
        return;

    m_width = static_cast<int>(width);
    m_height = static_cast<int>(height);

    DestroyRenderTargets();
    CreateRenderTargets();
}

////////////////////////////////////////////////////////////////////////////////
// Frame

void OpenGLRenderer::BeginScene()
{
    m_drawCallCount = 0;

    glBindFramebuffer(GL_FRAMEBUFFER, m_sceneFBO);
    glViewport(0, 0, m_width, m_height);
    glEnable(GL_DEPTH_TEST);

    const auto& fill = m_vertexConstants.Palette[m_fill_colour_idx];
    glClearColor(fill.x, fill.y, fill.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto aspect_ratio = static_cast<float>(m_width) / m_height;
    auto projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(m_verticalFOV), aspect_ratio, NEAR_CLIP, FAR_CLIP);
    m_mViewProjection = m_camera.GetViewMatrix() * projection;
    m_vertexConstants.EyePos = m_camera.GetPosition();

    // A new random offset each frame animates the dissolve noise.
    m_fRandom = (random_uint32() >> 8) / 16777216.0f;  // [0, 1) with full float precision
    m_pixelConstants.time = m_noise_enabled ? m_fRandom : 0.0f;
}

void OpenGLRenderer::Render(IGame* pGame)
{
    glUseProgram(m_sentinelProgram);
    pGame->Render(this);
    glBindVertexArray(0);
}

void OpenGLRenderer::EndScene()
{
    auto effects = PixelShaderEffectsActive();

    // Resolve the multisampled scene into the window, or into the effect texture.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_sceneFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, effects ? m_resolveFBO : 0);
    glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (effects)
    {
        // Dissolved pixels are discarded, leaving the fill colour behind them.
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);

        UpdateConstantBuffers();
        glUseProgram(m_effectProgram);
        glBindVertexArray(m_effectVao);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_resolveTexture);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindVertexArray(0);
    }

    glUseProgram(0);
    ReleaseExpiredMeshes();
    CheckGLError("OpenGLRenderer::EndScene");
}

void OpenGLRenderer::DrawModel(Model& model, const Model& linkedModel)
{
    if (!model)
        return;

    const auto& mesh = GetMesh(model);

    // DirectXMath matrices are row-major for row vectors. GLSL reads the same memory
    // as column-major, i.e. transposed, so "M * v" in the shaders matches "v * M" here.
    m_vertexConstants.W = model.GetWorldMatrix(linkedModel);
    m_vertexConstants.WVP = m_vertexConstants.W * (model.orthographic ? GetOrthographicMatrix() : m_mViewProjection);
    m_vertexConstants.lighting = model.lighting ? 1 : 0;
    m_pixelConstants.dissolved = model.dissolved;
    UpdateConstantBuffers();

    // The landscape is drawn double-sided so its underside is visible.
    if (model.type == ModelType::Landscape)
        glDisable(GL_CULL_FACE);
    else
        glEnable(GL_CULL_FACE);

    glBindVertexArray(mesh.vao);
    glDrawElements(GL_TRIANGLES, mesh.index_count, GL_UNSIGNED_INT, nullptr);
    ++m_drawCallCount;
}

void OpenGLRenderer::UpdateConstantBuffers()
{
    glBindBuffer(GL_UNIFORM_BUFFER, m_vertexConstantsUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(VertexConstants), &m_vertexConstants);
    glBindBuffer(GL_UNIFORM_BUFFER, m_pixelConstantsUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PixelConstants), &m_pixelConstants);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

////////////////////////////////////////////////////////////////////////////////
// Mesh cache

const OpenGLRenderer::Mesh& OpenGLRenderer::GetMesh(const Model& model)
{
    auto& mesh = m_meshes[{ model.m_pVertices.get(), model.m_pIndices.get() }];

    // New geometry, or a new vector at the address of one that has since been freed.
    if (!mesh.vao || mesh.vertices.expired() || mesh.indices.expired())
    {
        mesh.vertices = model.m_pVertices;
        mesh.indices = model.m_pIndices;
        Upload(mesh, model);
    }

    return mesh;
}

void OpenGLRenderer::Upload(Mesh& mesh, const Model& model)
{
    const auto& vertices = *model.m_pVertices;
    const auto& indices = *model.m_pIndices;

    if (!mesh.vao)
    {
        glGenVertexArrays(1, &mesh.vao);
        glGenBuffers(1, &mesh.vbo);
        glGenBuffers(1, &mesh.ibo);
    }

    glBindVertexArray(mesh.vao);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);
    mesh.index_count = static_cast<GLsizei>(indices.size());

    auto offset = [](size_t bytes) { return reinterpret_cast<const void*>(bytes); };
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), offset(offsetof(Vertex, pos)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), offset(offsetof(Vertex, normal)));
    glEnableVertexAttribArray(2);
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(Vertex), offset(offsetof(Vertex, colour)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), offset(offsetof(Vertex, texcoord)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void OpenGLRenderer::Destroy(Mesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(1, &mesh.vbo);
    glDeleteBuffers(1, &mesh.ibo);
    mesh = {};
}

void OpenGLRenderer::ReleaseExpiredMeshes()
{
    for (auto it = m_meshes.begin(); it != m_meshes.end();)
    {
        if (it->second.vertices.expired() || it->second.indices.expired())
        {
            Destroy(it->second);
            it = m_meshes.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// Camera

XMVECTOR OpenGLRenderer::GetEyePositionVector() const
{
    return m_camera.GetPositionVector();
}

XMVECTOR OpenGLRenderer::GetViewPositionVector() const
{
    return m_camera.GetPositionVector();
}

XMVECTOR OpenGLRenderer::GetViewDirectionVector() const
{
    return m_camera.GetDirectionVector();
}

XMVECTOR OpenGLRenderer::GetViewUpVector() const
{
    return m_camera.GetUpVector();
}

XMMATRIX OpenGLRenderer::GetViewProjectionMatrix() const
{
    return m_mViewProjection;
}

XMMATRIX OpenGLRenderer::GetOrthographicMatrix() const
{
    auto aspect_ratio = static_cast<float>(m_width) / m_height;
    return XMMatrixOrthographicOffCenterLH(0.0f, ORTHO_HEIGHT * aspect_ratio, 0.0f, ORTHO_HEIGHT, NEAR_CLIP, FAR_CLIP);
}

void OpenGLRenderer::GetSelectionRay(XMVECTOR& vPos, XMVECTOR& vDir) const
{
    vPos = m_camera.GetPositionVector();
    vDir = m_camera.GetDirectionVector();
}

void OpenGLRenderer::SetVerticalFOV(float fov)
{
    m_verticalFOV = fov;
}
