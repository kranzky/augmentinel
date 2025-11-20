#include "Platform.h"
#include "Audio.h"
#include "Utils.h"

Audio::Audio() {
    // Initialize SDL audio if not already done
    if (SDL_WasInit(SDL_INIT_AUDIO) == 0) {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
            SDL_Log("ERROR: Failed to initialize SDL audio: %s", SDL_GetError());
            return;
        }
    }

    // Initialize SDL_mixer
    int flags = MIX_INIT_MP3 | MIX_INIT_OGG;
    if ((Mix_Init(flags) & flags) != flags) {
        SDL_Log("WARNING: Mix_Init: Failed to init some audio formats: %s", Mix_GetError());
    }

    // Open audio device
    // 44100 Hz, default format, 2 channels (stereo), 2048 byte chunks
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        SDL_Log("ERROR: Failed to open audio device: %s", Mix_GetError());
        return;
    }

    // Allocate mixing channels
    Mix_AllocateChannels(16);

    m_initialized = true;

    // Set default sound directories (Amiga sounds by default)
    m_soundsDir = "sounds/Commodore Amiga";
    m_musicDir = "sounds/music";

    SDL_Log("Audio system initialized successfully");
    SDL_Log("  Sound effects: %s", m_soundsDir.string().c_str());
    SDL_Log("  Music: %s", m_musicDir.string().c_str());

    // Load and play background music automatically
    // Note: Some WAV formats (like Microsoft ADPCM) may not be supported by SDL_mixer
    fs::path musicPath = m_musicDir / "amiga.wav";
    if (fs::exists(musicPath)) {
        PlayMusic(musicPath, true);  // Loop music
        if (!m_musicPlaying) {
            SDL_Log("WARNING: Music loaded but failed to play (possibly unsupported format)");
            SDL_Log("WARNING: Consider converting to PCM format: ffmpeg -i input.wav -acodec pcm_s16le output.wav");
        }
    } else {
        SDL_Log("WARNING: Music file not found: %s", musicPath.string().c_str());
    }
}

Audio::~Audio() {
    Stop();

    // Free all loaded sounds
    for (auto& [name, chunk] : m_sounds) {
        if (chunk) {
            Mix_FreeChunk(chunk);
        }
    }
    m_sounds.clear();

    // Free music
    if (m_music) {
        Mix_FreeMusic(m_music);
        m_music = nullptr;
    }

    // Close audio
    if (m_initialized) {
        Mix_CloseAudio();
        Mix_Quit();
        SDL_Log("Audio system shut down");
    }
}

bool Audio::LoadWAV(const fs::path& path) {
    if (!m_initialized) return false;

    Mix_Chunk* chunk = Mix_LoadWAV(path.string().c_str());
    if (!chunk) {
        SDL_Log("ERROR: Failed to load sound '%s': %s", path.string().c_str(), Mix_GetError());
        return false;
    }

    // Store with the filename as key
    std::wstring filename = path.filename().wstring();
    m_sounds[filename] = chunk;
    SDL_Log("Loaded sound: %s", path.string().c_str());
    return true;
}

fs::path Audio::GetSoundPath(const std::wstring& filename) {
    // Convert wstring to string
    std::string filenameStr = to_string(filename);

    // Check if it's a full path or just a filename
    fs::path fullPath = m_soundsDir / filenameStr;
    if (fs::exists(fullPath)) {
        return fullPath;
    }

    // Try with .wav extension if not already present
    if (fullPath.extension() != ".wav") {
        fullPath.replace_extension(".wav");
        if (fs::exists(fullPath)) {
            return fullPath;
        }
    }

    // Return original if not found (will error when loading)
    return m_soundsDir / filenameStr;
}

Mix_Chunk* Audio::LoadSound(const std::wstring& filename) {
    // Check if already loaded
    if (m_sounds.count(filename)) {
        return m_sounds[filename];
    }

    // Load the sound
    fs::path soundPath = GetSoundPath(filename);
    if (LoadWAV(soundPath)) {
        return m_sounds[filename];
    }

    return nullptr;
}

bool Audio::Play(const std::wstring& filename, AudioType type) {
    if (!m_initialized) return false;

    Mix_Chunk* chunk = LoadSound(filename);
    if (!chunk) {
        return false;
    }

    // Play on first available channel, 0 loops (play once)
    int channel = Mix_PlayChannel(-1, chunk, 0);
    if (channel < 0) {
        SDL_Log("ERROR: Failed to play sound '%s': %s",
                to_string(filename).c_str(), Mix_GetError());
        return false;
    }

    // Set volume based on type
    Mix_Volume(channel, static_cast<int>(m_soundVolume * MIX_MAX_VOLUME));

    return true;
}

void Audio::Play(const std::wstring& filename, AudioType type, XMFLOAT3 pos) {
    // For now, just play without 3D positioning (spatial audio is optional)
    // TODO: Implement 3D spatial audio with SDL_mixer distance/panning
    Play(filename, type);
}

void Audio::Play(const std::wstring& filename) {
    Play(filename, AudioType::Effect);
}

void Audio::PlaySound(const fs::path& path, float volume) {
    if (!m_initialized) return;

    Mix_Chunk* chunk = Mix_LoadWAV(path.string().c_str());
    if (!chunk) {
        SDL_Log("ERROR: Failed to load sound '%s': %s",
                path.string().c_str(), Mix_GetError());
        return;
    }

    int channel = Mix_PlayChannel(-1, chunk, 0);
    if (channel >= 0) {
        Mix_Volume(channel, static_cast<int>(volume * MIX_MAX_VOLUME));
    }

    // Note: This leaks the chunk, but it's simple for one-off sounds
    // For production, should track and free after playback
}

void Audio::PlayMusic(const fs::path& path, bool loop) {
    if (!m_initialized) return;

    // Stop current music if playing
    if (m_music) {
        Mix_HaltMusic();
        Mix_FreeMusic(m_music);
        m_music = nullptr;
    }

    // Load new music
    m_music = Mix_LoadMUS(path.string().c_str());
    if (!m_music) {
        SDL_Log("ERROR: Failed to load music '%s': %s",
                path.string().c_str(), Mix_GetError());
        return;
    }

    // Set music volume
    Mix_VolumeMusic(static_cast<int>(m_musicVolume * MIX_MAX_VOLUME));

    // Play music (-1 for infinite loop, 0 for play once)
    if (Mix_PlayMusic(m_music, loop ? -1 : 0) < 0) {
        SDL_Log("ERROR: Failed to play music '%s': %s",
                path.string().c_str(), Mix_GetError());
        return;
    }

    m_musicPlaying = true;
    SDL_Log("Playing music: %s %s", path.string().c_str(), loop ? "(looping)" : "");
}

void Audio::SetMusicVolume(float volume) {
    m_musicVolume = std::clamp(volume, 0.0f, 1.0f);
    if (m_initialized) {
        Mix_VolumeMusic(static_cast<int>(m_musicVolume * MIX_MAX_VOLUME));
    }
}

bool Audio::SetMusicPlaying(bool play) {
    if (!m_initialized || !m_music) return false;

    if (play && !m_musicPlaying) {
        Mix_ResumeMusic();
        m_musicPlaying = true;
    } else if (!play && m_musicPlaying) {
        Mix_PauseMusic();
        m_musicPlaying = false;
    }

    return true;
}

void Audio::PositionListener(XMFLOAT3 pos, XMFLOAT3 dir, XMFLOAT3 up) {
    // Spatial audio is optional for Phase 4.1
    // Could implement with Mix_SetPosition, Mix_SetDistance, etc.
    // For now, just a stub
}

bool Audio::IsPlaying(AudioType type) const {
    if (!m_initialized) return false;

    if (type == AudioType::Music || type == AudioType::Tune) {
        return Mix_PlayingMusic() != 0;
    } else {
        // Check if any channels are playing
        return Mix_Playing(-1) > 0;
    }
}

void Audio::Stop(AudioType type) {
    if (!m_initialized) return;

    if (type == AudioType::Music || type == AudioType::Tune) {
        Mix_HaltMusic();
        m_musicPlaying = false;
    } else {
        // Stop all sound effects
        Mix_HaltChannel(-1);
    }
}

void Audio::Stop() {
    if (!m_initialized) return;

    Mix_HaltMusic();
    Mix_HaltChannel(-1);
    m_musicPlaying = false;
}
