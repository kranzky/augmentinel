#include "Platform.h"
#include "Application.h"
#include "OpenGLRenderer.h"
#include "Augmentinel.h"
#include "Settings.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

static constexpr float MAX_ACCUMULATED_TIME = 0.25f;

Application::Application()
{
}

Application::~Application()
{
    Shutdown();
}

bool Application::Init()
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) < 0)
    {
        SDL_Log("SDL initialization failed: %s", SDL_GetError());
        return false;
    }

    // Request OpenGL 3.3 Core Profile
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    // Mouse.
    SDL_SetHint(SDL_HINT_WINDOWS_DPI_SCALING, "1");
    SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_SCALING, "1");

    // MSAA
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    // Create window
    m_window = SDL_CreateWindow(
        APP_NAME " " APP_VERSION,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        m_windowWidth,
        m_windowHeight,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    if (!m_window)
    {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        return false;
    }

    // Create OpenGL context
    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext)
    {
        SDL_Log("OpenGL context creation failed: %s", SDL_GetError());
        return false;
    }

    // Enable VSync
    SDL_GL_SetSwapInterval(1);

    // Log OpenGL info
    SDL_Log("OpenGL Version: %s", glGetString(GL_VERSION));
    SDL_Log("GLSL Version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    SDL_Log("Renderer: %s", glGetString(GL_RENDERER));
    SDL_Log("Vendor: %s", glGetString(GL_VENDOR));

    // Set settings path to be alongside executable
    char* basePath = SDL_GetBasePath();
    if (basePath) {
        settings_path = std::wstring(basePath, basePath + strlen(basePath)) + L"settings.ini";
        SDL_free(basePath);
    } else {
        settings_path = L"settings.ini";
    }

    // Initialize settings
    InitSettings(APP_NAME);

    // Restore fullscreen state from settings
    m_fullscreen = GetFlag(L"Fullscreen", false);
    if (m_fullscreen)
    {
        m_windowedWidth = m_windowWidth;
        m_windowedHeight = m_windowHeight;
        SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        // Ensure window has input focus after going fullscreen
        SDL_RaiseWindow(m_window);
        SDL_SetWindowInputFocus(m_window);
        SDL_Log("Restored fullscreen mode from settings");
    }

    // Create renderer
    auto pOpenGLRenderer = std::make_shared<OpenGLRenderer>(m_windowWidth, m_windowHeight);
    if (!pOpenGLRenderer->Init())
    {
        SDL_Log("Renderer initialization failed");
        return false;
    }
    m_pRenderer = pOpenGLRenderer;

    // Create audio
    m_pAudio = std::make_shared<Audio>();

    // Restore sound pack from settings (stored as pack name string)
    std::wstring savedPackName = GetSetting(L"SoundPack", std::wstring(L"Commodore Amiga"));
    SoundPack savedPack = SoundPack::Amiga;
    if (savedPackName == L"Commodore 64") savedPack = SoundPack::C64;
    else if (savedPackName == L"BBC Micro") savedPack = SoundPack::BBC;
    else if (savedPackName == L"Sinclair ZX Spectrum") savedPack = SoundPack::Spectrum;
    m_pAudio->SetSoundPack(savedPack);

    // Create game
    m_pGame = std::make_unique<Augmentinel>(m_pRenderer, m_pAudio);

    // Create debug overlay
    m_pDebugOverlay = std::make_unique<DebugOverlay>();
    if (!m_pDebugOverlay->Init(m_windowWidth, m_windowHeight))
    {
        SDL_Log("WARNING: DebugOverlay init failed, debug overlay will be disabled");
        m_pDebugOverlay.reset();
    }

    // Enable relative mouse mode for free look
    SDL_SetRelativeMouseMode(SDL_TRUE);

    return true;
}

void Application::Run(bool dumpScreenshot)
{
    auto lastTime = std::chrono::high_resolution_clock::now();
    int warmupFrames = dumpScreenshot ? 10 : 0; // Wait 10 frames before screenshot

    // Enable debug info by default when taking screenshots
    m_showDebugInfo = dumpScreenshot;
    m_fpsLastTime = SDL_GetTicks();

    while (m_running)
    {
        // Process events
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ProcessEvent(event);
        }

        if (!m_running)
            break;

        // Calculate delta time
        auto currentTime = std::chrono::high_resolution_clock::now();
        float elapsed = std::chrono::duration<float>(currentTime - lastTime).count();
        elapsed = std::min(elapsed, MAX_ACCUMULATED_TIME);
        lastTime = currentTime;

        // Update FPS counter
        m_frameCount++;
        m_fpsFrameCount++;
        m_avgFrameTime = elapsed * 1000.0f; // Convert to milliseconds

        uint32_t currentTicks = SDL_GetTicks();
        if (currentTicks - m_fpsLastTime >= 1000)
        { // Update FPS every second
            m_currentFPS = m_fpsFrameCount * 1000.0f / (currentTicks - m_fpsLastTime);
            m_fpsFrameCount = 0;
            m_fpsLastTime = currentTicks;
        }

        // Update debug overlay every frame if enabled
        if (m_showDebugInfo && m_pDebugOverlay && m_pRenderer)
        {
            auto *glRenderer = dynamic_cast<OpenGLRenderer *>(m_pRenderer.get());

            std::vector<std::string> debugLines;
            char buffer[256];

            snprintf(buffer, sizeof(buffer), "FPS: %.1f", m_currentFPS);
            debugLines.push_back(buffer);

            snprintf(buffer, sizeof(buffer), "Frame Time: %.2f ms", m_avgFrameTime);
            debugLines.push_back(buffer);

            snprintf(buffer, sizeof(buffer), "Total Frames: %u", m_frameCount);
            debugLines.push_back(buffer);

            if (glRenderer)
            {
                snprintf(buffer, sizeof(buffer), "Draw Calls: %u", glRenderer->GetDrawCallCount());
                debugLines.push_back(buffer);

                snprintf(buffer, sizeof(buffer), "Uploaded Models: %u", glRenderer->GetModelCount());
                debugLines.push_back(buffer);
            }

            m_pDebugOverlay->SetText(debugLines);
        }

        // Update game
        if (m_pGame)
        {
            m_pGame->Frame(elapsed);

            // Check if game wants to quit (e.g., from title screen)
            if (m_pGame->WantsToQuit())
            {
                m_running = false;
                break;
            }
        }

        // Process key edges (convert DownEdge->Down, UpEdge->Up)
        if (m_pRenderer)
        {
            m_pRenderer->ProcessKeyEdges();
        }

        // Render
        if (m_pRenderer)
        {
            m_pRenderer->BeginScene();
            if (m_pGame)
            {
                m_pRenderer->Render(m_pGame.get());
            }
            m_pRenderer->EndScene();
        }

        // Render debug overlay on top of everything
        if (m_showDebugInfo && m_pDebugOverlay)
        {
            m_pDebugOverlay->Render();
        }

        // Dump screenshot BEFORE swap if requested (to capture back buffer)
        if (dumpScreenshot && warmupFrames > 0)
        {
            warmupFrames--;
        }
        if (dumpScreenshot && warmupFrames == 0)
        {
            // Display final performance stats before screenshot
            if (m_showDebugInfo && m_pRenderer)
            {
                auto *glRenderer = dynamic_cast<OpenGLRenderer *>(m_pRenderer.get());
                SDL_Log("=== Final Performance Stats ===");
                SDL_Log("  Total Frames: %u", m_frameCount);
                SDL_Log("  Avg Frame Time: %.2f ms", m_avgFrameTime);
                if (glRenderer)
                {
                    SDL_Log("  Draw Calls (last frame): %u", glRenderer->GetDrawCallCount());
                    SDL_Log("  Uploaded Models: %u", glRenderer->GetModelCount());
                }
                SDL_Log("===============================");
            }

            SDL_Log("Capturing screenshot...");

            // Allocate buffer for screenshot (RGB, no alpha)
            int width = m_windowWidth;
            int height = m_windowHeight;
            std::vector<uint8_t> pixels(width * height * 3);

            // Read pixels from back buffer (where we just rendered)
            glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

            // Flip image vertically (OpenGL has origin at bottom-left, image formats at top-left)
            std::vector<uint8_t> flipped(width * height * 3);
            for (int y = 0; y < height; y++)
            {
                memcpy(&flipped[y * width * 3], &pixels[(height - 1 - y) * width * 3], width * 3);
            }

            // Save as PNG
            const char *filename = "screenshot.png";
            if (stbi_write_png(filename, width, height, 3, flipped.data(), width * 3))
            {
                SDL_Log("Screenshot saved: %s (%dx%d)", filename, width, height);
            }
            else
            {
                SDL_Log("ERROR: Failed to save screenshot");
            }

            // Exit
            m_running = false;
            break;
        }

        // Swap buffers (after screenshot if needed)
        SDL_GL_SwapWindow(m_window);
    }
}

void Application::ProcessEvent(const SDL_Event &event)
{
    switch (event.type)
    {
    case SDL_QUIT:
        m_running = false;
        break;

    case SDL_KEYDOWN:
        ProcessKeyEvent(event.key, true);
        break;

    case SDL_KEYUP:
        ProcessKeyEvent(event.key, false);
        break;

    case SDL_MOUSEBUTTONDOWN:
        ProcessMouseButton(event.button, true);
        break;

    case SDL_MOUSEBUTTONUP:
        ProcessMouseButton(event.button, false);
        break;

    case SDL_MOUSEMOTION:
        if (m_pRenderer)
        {
            m_pRenderer->MouseMove(event.motion.xrel, event.motion.yrel);
        }
        break;

    case SDL_WINDOWEVENT:
        if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
        {
            m_windowWidth = event.window.data1;
            m_windowHeight = event.window.data2;
            if (m_pRenderer)
            {
                m_pRenderer->OnResize(m_windowWidth, m_windowHeight);
            }
            if (m_pDebugOverlay)
            {
                m_pDebugOverlay->OnResize(m_windowWidth, m_windowHeight);
            }
        }
        break;
    }
}

void Application::ProcessKeyEvent(const SDL_KeyboardEvent &key, bool pressed)
{
    // Ignore key repeat events - we only want the initial press
    // Key state will remain "Down" until released, which is what we want for movement
    // Action keys (ESC, TAB) check for DownEdge and will only trigger once
    if (key.repeat)
    {
        return;
    }

    // Special case: TAB to toggle debug info
    if (key.keysym.sym == SDLK_TAB && pressed)
    {
        m_showDebugInfo = !m_showDebugInfo;
        return;
    }

    // Special case: F11 or ALT+Enter to toggle fullscreen
    bool isFullscreenToggle = (key.keysym.sym == SDLK_F11) ||
                               (key.keysym.sym == SDLK_RETURN && (key.keysym.mod & KMOD_ALT));
    if (isFullscreenToggle && pressed)
    {
        m_fullscreen = !m_fullscreen;
        SetSetting(L"Fullscreen", m_fullscreen);
        if (m_fullscreen)
        {
            // Store windowed size before going fullscreen
            m_windowedWidth = m_windowWidth;
            m_windowedHeight = m_windowHeight;
            SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
            SDL_Log("Fullscreen mode enabled");
        }
        else
        {
            SDL_SetWindowFullscreen(m_window, 0);
            // Restore windowed size
            SDL_SetWindowSize(m_window, m_windowedWidth, m_windowedHeight);
            SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
            SDL_Log("Windowed mode enabled");
        }
        return;
    }

    // Special case: Number keys 1-4 to switch sound packs
    if (pressed && m_pAudio)
    {
        SoundPack newPack = m_pAudio->GetSoundPack();
        switch (key.keysym.sym)
        {
            case SDLK_1:
                newPack = SoundPack::Amiga;
                break;
            case SDLK_2:
                newPack = SoundPack::C64;
                break;
            case SDLK_3:
                newPack = SoundPack::BBC;
                break;
            case SDLK_4:
                newPack = SoundPack::Spectrum;
                break;
            default:
                break;
        }
        if (newPack != m_pAudio->GetSoundPack())
        {
            m_pAudio->SetSoundPack(newPack);
            // Save as pack name string (matches how Augmentinel.cpp reads it)
            SetSetting(L"SoundPack", std::wstring(to_wstring(m_pAudio->GetSoundPackName(newPack))));
            return;
        }
    }

    // Pass key events to renderer for game input (including ESC)
    if (m_pRenderer)
    {
        // SDL keycodes map directly to VK_ codes via Platform.h
        int virtKey = key.keysym.sym;
        m_pRenderer->UpdateKey(virtKey, pressed ? KeyState::DownEdge : KeyState::UpEdge);
    }
}

void Application::ProcessMouseButton(const SDL_MouseButtonEvent &button, bool pressed)
{
    if (m_pRenderer)
    {
        // Map SDL mouse buttons to VK_ codes (offset by 1000 as defined in Platform.h)
        int virtKey = 1000 + button.button;
        m_pRenderer->UpdateKey(virtKey, pressed ? KeyState::DownEdge : KeyState::UpEdge);
    }
}

void Application::Shutdown()
{
    SDL_Log("Shutting down...");

    m_pGame.reset();
    m_pAudio.reset();
    m_pRenderer.reset();

    if (m_glContext)
    {
        SDL_GL_DeleteContext(m_glContext);
        m_glContext = nullptr;
    }

    if (m_window)
    {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }

    SDL_Quit();
}
