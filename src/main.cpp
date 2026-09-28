#include "Platform.h"
#include "Application.h"

static void ShowError(const char* message)
{
    SDL_Log("%s", message);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, APP_NAME, message, nullptr);
}

static void ShowUsage(const char* program)
{
    SDL_Log("Usage: %s [--screenshot [file.png]] [--frames N] [--press FRAME:KEY]...", program);
    SDL_Log("  --screenshot, -s  Run N frames (default 10), save a PNG (default screenshot.png) and exit");
    SDL_Log("  --frames N        Frames to run before the screenshot");
    SDL_Log("  --press F:KEY     Tap KEY (an SDL key name, e.g. Space or Return) at frame F");
}

int main(int argc, char* argv[])
{
    Application::Script script;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        auto has_value = (i + 1 < argc && argv[i + 1][0] != '-');

        if (arg == "--screenshot" || arg == "-s")
        {
            script.screenshot_path = has_value ? fs::u8path(argv[++i]) : "screenshot.png";
        }
        else if (arg == "--frames" && has_value)
        {
            script.frames = std::max(1, std::atoi(argv[++i]));
        }
        else if (arg == "--press" && has_value)
        {
            std::string press = argv[++i];
            auto colon = press.find(':');
            auto key = (colon != std::string::npos) ? SDL_GetKeyFromName(press.substr(colon + 1).c_str()) : SDLK_UNKNOWN;
            if (key == SDLK_UNKNOWN)
            {
                SDL_Log("Invalid --press %s", press.c_str());
                return 1;
            }
            script.presses.emplace_back(std::atoi(press.c_str()), key);
        }
        else if (arg == "--help" || arg == "-h")
        {
            ShowUsage(argv[0]);
            return 0;
        }
        else if (arg.rfind("-psn_", 0) != 0)  // ignore the process serial number older macOS passes
        {
            SDL_Log("Unknown argument: %s", arg.c_str());
            ShowUsage(argv[0]);
            return 1;
        }
    }

    try
    {
        Application app;
        if (!app.Init())
        {
            ShowError("Augmentinel failed to start. It needs a graphics card supporting OpenGL 3.3.");
            return 1;
        }

        return app.Run(script) ? 0 : 1;
    }
    catch (const std::exception& e)
    {
        ShowError(e.what());
        return 1;
    }
}
