#include "Platform.h"
#include "Application.h"

int main(int argc, char* argv[]) {
    try {
        Application app;

        if (!app.Init()) {
            SDL_Log("Application initialization failed");
            return 1;
        }

        app.Run();

        return 0;
    }
    catch (const std::exception& e) {
        SDL_Log("Exception: %s", e.what());
        return 1;
    }
}
