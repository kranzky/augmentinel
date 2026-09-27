#include "Platform.h"
#include "DebugOverlay.h"

constexpr auto ROM_FONT_ADDRESS = 0x3d00;  // character set for 0x20-0x7f in the 48K ROM
constexpr auto CHAR_SIZE = 8;
constexpr auto BORDER = 4;                 // background margin around the text, in texels
constexpr uint32_t TEXT_COLOUR = 0xff00ffff;        // opaque yellow (ABGR, i.e. RGBA bytes)
constexpr uint32_t BACKGROUND_COLOUR = 0xb0000000;  // translucent black

// A textured quad placed by u_rect (x, y, width, height in clip space), generated
// from gl_VertexID as a four-vertex triangle strip.
static const char* OVERLAY_VERTEX_SHADER = R"(#version 330 core
uniform vec4 u_rect;
out vec2 v_texcoord;
void main()
{
    vec2 corner = vec2(gl_VertexID / 2, gl_VertexID % 2);
    gl_Position = vec4(u_rect.xy + corner * u_rect.zw, 0.0, 1.0);
    v_texcoord = vec2(corner.x, 1.0 - corner.y);
}
)";

static const char* OVERLAY_FRAGMENT_SHADER = R"(#version 330 core
uniform sampler2D u_texture;
in vec2 v_texcoord;
out vec4 frag_colour;
void main()
{
    frag_colour = texture(u_texture, v_texcoord);
}
)";

static GLuint CompileShader(GLenum type, const char* source)
{
    auto shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint status = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

DebugOverlay::~DebugOverlay()
{
    glDeleteTextures(1, &m_texture);
    glDeleteVertexArrays(1, &m_vao);
    glDeleteProgram(m_program);
}

bool DebugOverlay::Init(const fs::path& rom_path)
{
    auto rom = FileContents(rom_path);
    if (rom.size() < ROM_FONT_ADDRESS + m_font.size())
        return false;
    std::copy_n(rom.begin() + ROM_FONT_ADDRESS, m_font.size(), m_font.begin());

    auto vs = CompileShader(GL_VERTEX_SHADER, OVERLAY_VERTEX_SHADER);
    auto fs = CompileShader(GL_FRAGMENT_SHADER, OVERLAY_FRAGMENT_SHADER);
    if (vs && fs)
    {
        m_program = glCreateProgram();
        glAttachShader(m_program, vs);
        glAttachShader(m_program, fs);
        glLinkProgram(m_program);
    }
    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint status = GL_FALSE;
    if (m_program)
        glGetProgramiv(m_program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE)
        return false;

    m_rectLocation = glGetUniformLocation(m_program, "u_rect");
    glUseProgram(m_program);
    glUniform1i(glGetUniformLocation(m_program, "u_texture"), 0);
    glUseProgram(0);

    glGenVertexArrays(1, &m_vao);
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}

void DebugOverlay::SetText(const std::vector<std::string>& lines)
{
    if (lines != m_lines)
    {
        m_lines = lines;
        m_dirty = true;
    }
}

void DebugOverlay::Render(int width, int height)
{
    if (m_lines.empty() || !m_program)
        return;

    if (m_dirty)
    {
        // Rasterise the text into an RGBA image, top row first.
        size_t columns = 0;
        for (auto& line : m_lines)
            columns = std::max(columns, line.size());

        m_textureWidth = static_cast<int>(columns) * CHAR_SIZE + BORDER * 2;
        m_textureHeight = static_cast<int>(m_lines.size()) * CHAR_SIZE + BORDER * 2;
        std::vector<uint32_t> pixels(m_textureWidth * m_textureHeight, BACKGROUND_COLOUR);

        for (size_t row = 0; row < m_lines.size(); ++row)
        {
            for (size_t column = 0; column < m_lines[row].size(); ++column)
            {
                auto ch = static_cast<unsigned char>(m_lines[row][column]);
                if (ch < 0x20 || ch > 0x7f)
                    ch = '?';

                auto glyph = &m_font[(ch - 0x20) * CHAR_SIZE];
                for (int y = 0; y < CHAR_SIZE; ++y)
                {
                    for (int x = 0; x < CHAR_SIZE; ++x)
                    {
                        if (glyph[y] & (0x80 >> x))
                        {
                            auto px = BORDER + static_cast<int>(column) * CHAR_SIZE + x;
                            auto py = BORDER + static_cast<int>(row) * CHAR_SIZE + y;
                            pixels[py * m_textureWidth + px] = TEXT_COLOUR;
                        }
                    }
                }
            }
        }

        glBindTexture(GL_TEXTURE_2D, m_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_textureWidth, m_textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
        glBindTexture(GL_TEXTURE_2D, 0);
        m_dirty = false;
    }

    // Whole-pixel scaling keeps the font crisp: 2x at 720p, more on larger displays.
    auto scale = std::max(1, height / 360);
    auto margin = 4.0f * scale;
    auto rect_width = 2.0f * m_textureWidth * scale / width;
    auto rect_height = 2.0f * m_textureHeight * scale / height;
    auto left = -1.0f + 2.0f * margin / width;
    auto top = 1.0f - 2.0f * margin / height;

    glViewport(0, 0, width, height);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(m_program);
    glUniform4f(m_rectLocation, left, top - rect_height, rect_width, rect_height);
    glBindVertexArray(m_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
    glDisable(GL_BLEND);
}
