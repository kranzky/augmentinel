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

    int m_width;
    int m_height;

    // OpenGL objects (will be populated in Phase 2)
    GLuint m_vao{0};
    GLuint m_testVBO{0};  // Test triangle vertex buffer (Phase 2.9)
    GLuint m_sentinelProgram{0};
    GLuint m_effectProgram{0};
    GLuint m_vertexConstantsUBO{0};
    GLuint m_pixelConstantsUBO{0};

    // Model buffer management (Phase 3.3)
    std::map<const Model*, GLuint> m_modelVBOs;
    std::map<const Model*, GLuint> m_modelIBOs;
    std::map<const Model*, size_t> m_modelIndexCounts;
};
