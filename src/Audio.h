#pragma once
#include "Platform.h"

enum class AudioType {
    Tune,
    Music,
    LoopingEffect,
    Effect
};

// Stub Audio class for Phase 1
// Will be properly implemented with SDL2_mixer in Phase 4
class Audio {
public:
    Audio() {
        SDL_Log("Audio stub initialized");
    }

    ~Audio() {
    }

    bool Available() const { return false; }

    bool LoadWAV(const fs::path& path) {
        return false;
    }

    bool Play(const std::wstring& filename, AudioType type) {
        return false;
    }

    void Play(const std::wstring& filename, AudioType type, XMFLOAT3 pos) {
    }

    void Play(const std::wstring& filename) {
    }

    void PlaySound(const fs::path& path, float volume = 1.0f) {
    }

    void PlayMusic(const fs::path& path, bool loop = false) {
    }

    void SetMusicVolume(float volume) {
    }

    bool SetMusicPlaying(bool play) {
        return false;
    }

    void PositionListener(XMFLOAT3 pos, XMFLOAT3 dir, XMFLOAT3 up) {
    }

    bool IsPlaying(AudioType type) const {
        return false;
    }

    void Stop(AudioType type) {
    }

    void Stop() {
    }
};
