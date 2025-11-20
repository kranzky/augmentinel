#pragma once
#include "Platform.h"
#include <map>
#include <string>

enum class AudioType {
    Tune,
    Music,
    LoopingEffect,
    Effect
};

// Audio class using SDL2_mixer
class Audio {
public:
    Audio();
    ~Audio();

    bool Available() const { return m_initialized; }

    bool LoadWAV(const fs::path& path);

    bool Play(const std::wstring& filename, AudioType type);
    void Play(const std::wstring& filename, AudioType type, XMFLOAT3 pos);
    void Play(const std::wstring& filename);
    void PlaySound(const fs::path& path, float volume = 1.0f);
    void PlayMusic(const fs::path& path, bool loop = false);

    void SetMusicVolume(float volume);
    bool SetMusicPlaying(bool play);

    void PositionListener(XMFLOAT3 pos, XMFLOAT3 dir, XMFLOAT3 up);

    bool IsPlaying(AudioType type) const;

    void Stop(AudioType type);
    void Stop();

private:
    bool m_initialized{false};

    // SDL_mixer types
    Mix_Music* m_music{nullptr};
    std::map<std::wstring, Mix_Chunk*> m_sounds;

    // Audio state
    float m_musicVolume{1.0f};
    float m_soundVolume{1.0f};
    bool m_musicPlaying{false};

    // Sound directory paths
    fs::path m_soundsDir;
    fs::path m_musicDir;

    // Helper methods
    fs::path GetSoundPath(const std::wstring& filename);
    Mix_Chunk* LoadSound(const std::wstring& filename);
};
