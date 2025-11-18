#pragma once
#include "Platform.h"
#include "Game.h"
#include "Audio.h"
#include "View.h"

class Application {
public:
    Application();
    ~Application();

    bool Init();
    void Run();
    void Shutdown();

private:
    void ProcessEvent(const SDL_Event& event);
    void ProcessKeyEvent(const SDL_KeyboardEvent& key, bool pressed);
    void ProcessMouseButton(const SDL_MouseButtonEvent& button, bool pressed);

    SDL_Window* m_window{nullptr};
    SDL_GLContext m_glContext{nullptr};

    std::shared_ptr<View> m_pRenderer;
    std::shared_ptr<Audio> m_pAudio;
    std::unique_ptr<Game> m_pGame;

    bool m_running{true};
    int m_windowWidth{1600};
    int m_windowHeight{900};
};
