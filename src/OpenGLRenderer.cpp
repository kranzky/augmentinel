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
    // For Phase 1, just set up basic OpenGL state
    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);  // Dark blue background for testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    SDL_Log("OpenGLRenderer initialized");
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
