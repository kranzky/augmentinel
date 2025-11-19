#include "Platform.h"
#include "Application.h"

int main(int argc, char* argv[]) {
    try {
        // Parse command-line arguments
        bool dumpScreenshot = false;
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "--screenshot" || arg == "-s") {
                dumpScreenshot = true;
            } else if (arg == "--help" || arg == "-h") {
                SDL_Log("Usage: %s [options]", argv[0]);
                SDL_Log("Options:");
                SDL_Log("  --screenshot, -s   Render one frame, save screenshot.png, and exit");
                SDL_Log("  --help, -h         Show this help message");
                return 0;
            } else {
                SDL_Log("Unknown argument: %s", arg.c_str());
                SDL_Log("Use --help for usage information");
                return 1;
            }
        }

        Application app;

        if (!app.Init()) {
            SDL_Log("Application initialization failed");
            return 1;
        }

        app.Run(dumpScreenshot);

        return 0;
    }
    catch (const std::exception& e) {
        SDL_Log("Exception: %s", e.what());
        return 1;
    }
}
