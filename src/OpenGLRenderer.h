#pragma once
#include "Platform.h"
#include "GL.h"
#include "View.h"

// Flat-screen OpenGL 3.3 renderer. The scene is drawn into a multisampled
// framebuffer, then resolved to the window directly or through the effect shader
// (dissolve, desaturate, fade) when any view effect is active.
class OpenGLRenderer : public View
{
public:
    OpenGLRenderer() = default;
    ~OpenGLRenderer() override;
    OpenGLRenderer(const OpenGLRenderer&) = delete;
    OpenGLRenderer& operator=(const OpenGLRenderer&) = delete;

    // Sizes are drawable sizes in pixels, which differ from window sizes on HiDPI displays.
    bool Init(int width, int height);
    void OnResize(uint32_t width, uint32_t height) override;

    void BeginScene() override;
    void Render(IGame* pGame) override;
    void EndScene() override;
    void DrawModel(Model& model, const Model& linkedModel = {}) override;
    bool IsPointerVisible() const override { return false; }

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

    uint32_t GetDrawCallCount() const { return m_drawCallCount; }
    size_t GetMeshCount() const { return m_meshes.size(); }

private:
    // GPU copy of a model's geometry. Models share geometry through shared_ptrs, so
    // meshes are keyed by those vectors; the weak_ptrs detect when they are freed
    // (and their addresses may be reused), so stale meshes are never drawn.
    struct Mesh
    {
        std::weak_ptr<std::vector<Vertex>> vertices;
        std::weak_ptr<std::vector<uint32_t>> indices;
        GLuint vao{ 0 };
        GLuint vbo{ 0 };
        GLuint ibo{ 0 };
        GLsizei index_count{ 0 };
    };
    using MeshKey = std::pair<const void*, const void*>;

    const Mesh& GetMesh(const Model& model);
    static void Upload(Mesh& mesh, const Model& model);
    static void Destroy(Mesh& mesh);
    void ReleaseExpiredMeshes();

    bool CreateRenderTargets();
    void DestroyRenderTargets();
    void UpdateConstantBuffers();

    int m_width{ 0 };
    int m_height{ 0 };
    int m_samples{ 1 };
    float m_verticalFOV{ SENTINEL_VERT_FOV };

    GLuint m_sentinelProgram{ 0 };
    GLuint m_effectProgram{ 0 };
    GLuint m_effectVao{ 0 };
    GLuint m_vertexConstantsUBO{ 0 };
    GLuint m_pixelConstantsUBO{ 0 };

    GLuint m_sceneFBO{ 0 };
    GLuint m_sceneColourRBO{ 0 };
    GLuint m_sceneDepthRBO{ 0 };
    GLuint m_resolveFBO{ 0 };
    GLuint m_resolveTexture{ 0 };

    std::map<MeshKey, Mesh> m_meshes;
    uint32_t m_drawCallCount{ 0 };
};
