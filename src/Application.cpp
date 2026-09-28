#include "Platform.h"
#include "Application.h"
#include "Audio.h"
#include "Augmentinel.h"
#include "DebugOverlay.h"
#include "OpenGLRenderer.h"
#include "Settings.h"

#include "stb_image_write.h"

constexpr auto DEFAULT_WINDOW_WIDTH = 1600;
constexpr auto DEFAULT_WINDOW_HEIGHT = 900;
constexpr auto MAX_ACCUMULATED_TIME = 0.25f;  // longest frame time the game sees, in seconds

static constexpr auto FULLSCREEN_KEY{ L"Fullscreen" };
static constexpr auto SOUND_PACK_KEY{ L"SoundPack" };

fs::path g_resourcePath;

Application::Application() = default;

Application::~Application()
{
    // Everything holding GL objects must go before the context does.
    m_pGame.reset();
    m_pDebugOverlay.reset();
    m_pRenderer.reset();
    m_pAudio.reset();

    if (m_glContext)
        SDL_GL_DeleteContext(m_glContext);
    if (m_window)
        SDL_DestroyWindow(m_window);

    SDL_Quit();
}

// Resources are found beside the executable, or in Contents/Resources of a macOS app
// bundle. Settings live in the per-user app data folder, so they survive updates and
// work when the app folder is read-only (and never modify a signed bundle).
void Application::InitPaths()
{
    fs::path base_path = fs::current_path();
    if (auto base = SDL_GetBasePath())
    {
        base_path = fs::u8path(base);
        SDL_free(base);
    }

    auto bundle_resources = (base_path / ".." / "Resources").lexically_normal();
    g_resourcePath = fs::exists(bundle_resources / "48.rom") ? bundle_resources : base_path;

    fs::path settings_dir = base_path;
    if (auto pref = SDL_GetPrefPath("", APP_NAME))
    {
        settings_dir = fs::u8path(pref);
        SDL_free(pref);
    }
    auto settings_path = settings_dir / "settings.ini";

    // Adopt settings from earlier versions: beside the executable (1.6.x), or the
    // original Windows release's AppData\Augmentinel.ini.
    std::error_code ec;
    if (!fs::exists(settings_path, ec))
    {
        for (auto& old_path : { base_path / "settings.ini", settings_dir.parent_path().parent_path() / "Augmentinel.ini" })
        {
            if (fs::exists(old_path, ec) && fs::copy_file(old_path, settings_path, ec))
            {
                SDL_Log("Imported settings from %s", old_path.u8string().c_str());
                break;
            }
        }
    }

    InitSettings(settings_path);
}

bool Application::Init()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) < 0)
    {
        SDL_Log("SDL initialisation failed: %s", SDL_GetError());
        return false;
    }

    InitPaths();

    // Anti-aliasing is done by the renderer's own framebuffer, so the window's
    // framebuffer needs no multisampling, depth or stencil.
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);

    SDL_SetHint(SDL_HINT_WINDOWS_DPI_SCALING, "1");
    SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_SCALING, "1");

    m_window = SDL_CreateWindow(APP_NAME " v" APP_VERSION,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!m_window)
    {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        return false;
    }

    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext || !LoadGL())
    {
        SDL_Log("OpenGL 3.3 is required: %s", SDL_GetError());
        return false;
    }

    SDL_GL_SetSwapInterval(1);
    SDL_Log("OpenGL %s, GLSL %s, %s", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION), glGetString(GL_RENDERER));

    m_fullscreen = GetFlag(FULLSCREEN_KEY, false);
    if (m_fullscreen)
        SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP);

    SDL_GL_GetDrawableSize(m_window, &m_drawableWidth, &m_drawableHeight);
    m_pRenderer = std::make_shared<OpenGLRenderer>();
    if (!m_pRenderer->Init(m_drawableWidth, m_drawableHeight))
    {
        SDL_Log("Renderer initialisation failed");
        return false;
    }

    m_pAudio = std::make_shared<Audio>();
    m_pAudio->SetSoundPack(Audio::SoundPackFromName(to_string(GetSetting(SOUND_PACK_KEY, std::wstring{}))));

    std::shared_ptr<View> pView = m_pRenderer;
    m_pGame = std::make_unique<Augmentinel>(pView, m_pAudio);

    m_pDebugOverlay = std::make_unique<DebugOverlay>();
    if (!m_pDebugOverlay->Init(g_resourcePath / "48.rom"))
    {
        SDL_Log("Debug overlay unavailable");
        m_pDebugOverlay.reset();
    }

    // Take keyboard focus, which a window made fullscreen at startup may not get.
    SDL_RaiseWindow(m_window);
    SDL_SetWindowInputFocus(m_window);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    return true;
}

bool Application::Run(const Script& script)
{
    auto scripted = !script.screenshot_path.empty();
    m_showDebugInfo = scripted;

    auto last_time = std::chrono::steady_clock::now();
    m_fpsLastTicks = SDL_GetTicks();

    for (int frame = 1; m_running; ++frame)
    {
        // Scripted taps deliver press and release together, like a quick click.
        for (auto& [press_frame, key] : script.presses)
        {
            if (press_frame == frame)
            {
                for (auto type : { SDL_KEYDOWN, SDL_KEYUP })
                {
                    SDL_Event event{};
                    event.key.type = type;
                    event.key.state = (type == SDL_KEYDOWN) ? SDL_PRESSED : SDL_RELEASED;
                    event.key.keysym.sym = key;
                    SDL_PushEvent(&event);
                }
            }
        }

        SDL_Event event;
        while (SDL_PollEvent(&event))
            ProcessEvent(event);

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::min(std::chrono::duration<float>(now - last_time).count(), MAX_ACCUMULATED_TIME);
        last_time = now;

        m_pGame->Frame(elapsed);
        m_pRenderer->EndInputFrame();
        if (!m_running || m_pGame->WantsToQuit())
            break;

        m_pRenderer->BeginScene();
        m_pRenderer->Render(m_pGame.get());
        m_pRenderer->EndScene();

        UpdateDebugOverlay(elapsed);
        if (m_showDebugInfo && m_pDebugOverlay)
            m_pDebugOverlay->Render(m_drawableWidth, m_drawableHeight);

        // Capture from the back buffer, before it's presented.
        if (scripted && frame >= script.frames)
            return SaveScreenshot(script.screenshot_path);

        SDL_GL_SwapWindow(m_window);
    }

    return true;
}

void Application::ProcessEvent(const SDL_Event& event)
{
    switch (event.type)
    {
    case SDL_QUIT:
        m_running = false;
        break;

    case SDL_KEYDOWN:
    case SDL_KEYUP:
        ProcessKeyEvent(event.key);
        break;

    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
        m_pRenderer->UpdateKey(VK_MOUSE_BASE + event.button.button,
            (event.type == SDL_MOUSEBUTTONDOWN) ? KeyState::DownEdge : KeyState::UpEdge);
        break;

    case SDL_MOUSEMOTION:
        m_pRenderer->MouseMove(event.motion.xrel, event.motion.yrel);
        break;

    case SDL_WINDOWEVENT:
        if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            OnResize();
        else if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
            m_pRenderer->ReleaseKeys();  // key releases while unfocused are never seen
        break;
    }
}

void Application::ProcessKeyEvent(const SDL_KeyboardEvent& key)
{
    // Held keys are tracked by state, so auto-repeat events are ignored.
    if (key.repeat)
        return;

    auto pressed = (key.state == SDL_PRESSED);
    auto sym = key.keysym.sym;

    if (pressed)
    {
        if (sym == SDLK_TAB)
        {
            m_showDebugInfo = !m_showDebugInfo;
            return;
        }

        if (sym == SDLK_F11 || (sym == SDLK_RETURN && (key.keysym.mod & KMOD_ALT)))
        {
            ToggleFullscreen();
            return;
        }

        // 1-4 select the Amiga, C64, BBC or Spectrum sound pack.
        if (sym >= SDLK_1 && sym <= SDLK_4)
        {
            auto pack = static_cast<SoundPack>(sym - SDLK_1);
            m_pAudio->SetSoundPack(pack);
            SetSetting(SOUND_PACK_KEY, to_wstring(Audio::SoundPackName(pack)));
            return;
        }
    }

    m_pRenderer->UpdateKey(sym, pressed ? KeyState::DownEdge : KeyState::UpEdge);
}

void Application::ToggleFullscreen()
{
    m_fullscreen = !m_fullscreen;
    SDL_SetWindowFullscreen(m_window, m_fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    SetSetting(FULLSCREEN_KEY, m_fullscreen);
}

void Application::OnResize()
{
    SDL_GL_GetDrawableSize(m_window, &m_drawableWidth, &m_drawableHeight);
    m_pRenderer->OnResize(m_drawableWidth, m_drawableHeight);
}

void Application::UpdateDebugOverlay(float elapsed)
{
    ++m_frameCount;
    ++m_fpsFrameCount;

    auto ticks = SDL_GetTicks();
    if (ticks - m_fpsLastTicks >= 1000)
    {
        m_fps = m_fpsFrameCount * 1000.0f / (ticks - m_fpsLastTicks);
        m_fpsFrameCount = 0;
        m_fpsLastTicks = ticks;
    }

    if (!m_showDebugInfo || !m_pDebugOverlay)
        return;

    char fps[32], frame_time[32], frames[32], draws[32], meshes[32];
    snprintf(fps, sizeof(fps), "FPS: %.1f", m_fps);
    snprintf(frame_time, sizeof(frame_time), "Frame: %.2f ms", elapsed * 1000.0f);
    snprintf(frames, sizeof(frames), "Frames: %u", m_frameCount);
    snprintf(draws, sizeof(draws), "Draws: %u", m_pRenderer->GetDrawCallCount());
    snprintf(meshes, sizeof(meshes), "Meshes: %zu", m_pRenderer->GetMeshCount());
    m_pDebugOverlay->SetText({ fps, frame_time, frames, draws, meshes });
}

bool Application::SaveScreenshot(const fs::path& path) const
{
    auto width = m_drawableWidth, height = m_drawableHeight;
    auto stride = width * 3;
    std::vector<uint8_t> pixels(static_cast<size_t>(stride) * height);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // OpenGL rows run bottom-up; PNG rows run top-down.
    stbi_flip_vertically_on_write(1);
    if (!stbi_write_png(path.u8string().c_str(), width, height, 3, pixels.data(), stride))
    {
        SDL_Log("Failed to save screenshot %s", path.u8string().c_str());
        return false;
    }

    SDL_Log("Saved screenshot %s (%dx%d, %u draw calls)", path.u8string().c_str(), width, height, m_pRenderer->GetDrawCallCount());
    return true;
}
