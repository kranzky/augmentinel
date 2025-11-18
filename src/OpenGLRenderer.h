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
    int m_width;
    int m_height;

    // OpenGL objects (will be populated in Phase 2)
    GLuint m_vao{0};
    GLuint m_sentinelProgram{0};
    GLuint m_effectProgram{0};
    GLuint m_vertexConstantsUBO{0};
    GLuint m_pixelConstantsUBO{0};
};
