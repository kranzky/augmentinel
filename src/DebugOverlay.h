#pragma once
#include "Platform.h"
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>

class DebugOverlay {
public:
    DebugOverlay();
    ~DebugOverlay();

    bool Init(int windowWidth, int windowHeight);
    void SetText(const std::vector<std::string>& lines);
    void Render();
    void OnResize(int windowWidth, int windowHeight);

private:
    struct TextLine {
        GLuint texture{0};
        int width{0};
        int height{0};
        std::string text;
    };

    void UpdateTexture(TextLine& line, const std::string& text);
    void RenderQuad(float x, float y, float width, float height, GLuint texture);

    TTF_Font* m_font{nullptr};
    std::vector<TextLine> m_lines;
    int m_windowWidth{0};
    int m_windowHeight{0};

    // OpenGL resources for overlay rendering
    GLuint m_overlayProgram{0};
    GLuint m_overlayVAO{0};
    GLuint m_overlayVBO{0};

    SDL_Color m_textColor{255, 255, 0, 255};  // Yellow text
    SDL_Color m_bgColor{0, 0, 0, 200};        // Semi-transparent black background
};
