#pragma once
#include "Platform.h"
#include "GL.h"

// Performance stats drawn over the game (toggled with TAB), using the Spectrum
// ROM's own 8x8 character set.
class DebugOverlay
{
public:
    DebugOverlay() = default;
    ~DebugOverlay();
    DebugOverlay(const DebugOverlay&) = delete;
    DebugOverlay& operator=(const DebugOverlay&) = delete;

    bool Init(const fs::path& rom_path);
    void SetText(const std::vector<std::string>& lines);
    void Render(int width, int height);

private:
    std::array<uint8_t, 96 * 8> m_font{};  // characters 0x20 to 0x7f
    std::vector<std::string> m_lines;
    int m_textureWidth{ 0 };
    int m_textureHeight{ 0 };
    bool m_dirty{ false };

    GLuint m_program{ 0 };
    GLuint m_vao{ 0 };
    GLuint m_texture{ 0 };
    GLint m_rectLocation{ -1 };
};
