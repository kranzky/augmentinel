#pragma once
#include "Platform.h"
#include "Game.h"

class Audio;
class DebugOverlay;
class OpenGLRenderer;

class Application
{
public:
    Application();
    ~Application();
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    bool Init();

    // Scripted input and capture for automated smoke tests.
    struct Script
    {
        fs::path screenshot_path;                 // save the last frame here, then exit
        int frames{ 10 };                         // frames to run before the screenshot
        std::vector<std::pair<int, SDL_Keycode>> presses;  // taps: frame number, key
    };

    // Run until quit, or until the script's screenshot is taken. Returns false if
    // the screenshot couldn't be saved.
    bool Run(const Script& script);

private:
    void InitPaths();
    void ProcessEvent(const SDL_Event& event);
    void ProcessKeyEvent(const SDL_KeyboardEvent& key);
    void ToggleFullscreen();
    void OnResize();
    void UpdateDebugOverlay(float elapsed);
    bool SaveScreenshot(const fs::path& path) const;

    SDL_Window* m_window{ nullptr };
    SDL_GLContext m_glContext{ nullptr };

    std::shared_ptr<OpenGLRenderer> m_pRenderer;
    std::shared_ptr<Audio> m_pAudio;
    std::unique_ptr<Game> m_pGame;
    std::unique_ptr<DebugOverlay> m_pDebugOverlay;

    int m_drawableWidth{ 0 };
    int m_drawableHeight{ 0 };
    bool m_running{ true };
    bool m_fullscreen{ false };

    bool m_showDebugInfo{ false };
    uint32_t m_frameCount{ 0 };
    uint32_t m_fpsFrameCount{ 0 };
    uint32_t m_fpsLastTicks{ 0 };
    float m_fps{ 0.0f };
};
