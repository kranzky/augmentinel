#pragma once
#include "Platform.h"
#include "View.h"

class OpenGLRenderer : public View {
public:
    OpenGLRenderer(int width, int height);
    ~OpenGLRenderer() override;

    bool Init();

    // View interface - pure virtuals that must be implemented
    void BeginScene() override;
    void Render(IGame* pGame) override;
    void EndScene() override;

    void DrawModel(Model& model, const Model& linkedModel = {}) override;
    void DrawControllers() override;  // Stub for VR (not used in flat mode)
    bool IsPointerVisible() const override;

    void OnResize(uint32_t width, uint32_t height) override;
    void MouseMove(int x, int y) override;

    XMVECTOR GetEyePositionVector() const override;
    XMVECTOR GetViewPositionVector() const override;
    XMVECTOR GetViewDirectionVector() const override;
    XMVECTOR GetViewUpVector() const override;
    XMMATRIX GetViewProjectionMatrix() const override;
    XMMATRIX GetOrthographicMatrix() const override;
    void GetSelectionRay(XMVECTOR& vPos, XMVECTOR& vDir) const override;
    int GetWidth() const override { return m_width; }
    int GetHeight() const override { return m_height; }

    void SetVerticalFOV(float fov) override;

    // Performance stats
    uint32_t GetDrawCallCount() const { return m_drawCallCount; }
    uint32_t GetModelCount() const { return m_modelVBOs.size(); }
    void ResetStats() { m_drawCallCount = 0; }

    // Model cache management
    void ClearModelCache();

private:
    // Shader loading helpers
    std::string LoadShaderFile(const std::string& filename);
    GLuint CompileShader(const char* source, GLenum type, const char* name);
    GLuint LinkProgram(GLuint vs, GLuint fs, const char* name);

    // Uniform buffer update helpers
    void UpdateVertexConstants();
    void UpdatePixelConstants();

    // Model upload helper
    void UploadModel(const Model& model);

    // Cache key computation helper
    const void* ComputeCacheKey(const Model& model);

    // Framebuffer management (Phase 4.5)
    void InitFramebuffers();
    void ResizeFramebuffers();

    int m_width;
    int m_height;
    float m_verticalFOV{17.4f};  // Default Sentinel FOV

    // OpenGL objects
    GLuint m_vao{0};
    GLuint m_sentinelProgram{0};
    GLuint m_effectProgram{0};
    GLuint m_vertexConstantsUBO{0};
    GLuint m_pixelConstantsUBO{0};

    // Framebuffer objects for post-processing (Phase 4.5)
    GLuint m_sceneFBO{0};           // Framebuffer for rendering scene
    GLuint m_sceneTexture{0};       // Color texture attached to FBO
    GLuint m_sceneDepthRBO{0};      // Depth/stencil renderbuffer

    // Model buffer management (Phase 3.3)
    // Use vertex buffer pointer as cache key (identifies unique geometry)
    std::map<const void*, GLuint> m_modelVBOs;
    std::map<const void*, GLuint> m_modelIBOs;
    std::map<const void*, size_t> m_modelIndexCounts;

    // Performance tracking
    uint32_t m_drawCallCount{0};
};
