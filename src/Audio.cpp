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

    // Reserve channel 0 for looping effects (protects it from being used by other sounds)
    Mix_ReserveChannels(1);

    m_initialized = true;

    // Set default sound directories (Amiga sounds by default)
    m_soundsDir = "sounds/Commodore Amiga";
    m_musicDir = "sounds/music";

    // Load and play background music automatically
    // Using MP3 format (compressed, efficient)
    fs::path musicPath = m_musicDir / "amiga.mp3";
    if (fs::exists(musicPath)) {
        PlayMusic(musicPath, true);  // Loop music
        if (!m_musicPlaying) {
            SDL_Log("WARNING: Music loaded but failed to play");
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

    // Handle Music type - should use PlayMusic() instead
    if (type == AudioType::Music) {
        SDL_Log("WARNING: AudioType::Music should use PlayMusic(), not Play()");
        return false;
    }

    Mix_Chunk* chunk = LoadSound(filename);
    if (!chunk) {
        return false;
    }

    // Get appropriate channel and loop count for this audio type
    int channel;
    int loops;

    switch (type) {
        case AudioType::Tune:
            // Tunes: Play on dedicated tune channels (1-4), no loop
            channel = GetChannelForType(type);
            loops = 0;  // Play once
            break;

        case AudioType::LoopingEffect:
            // Looping effects: Play on channel 0, infinite loop
            channel = LOOPING_EFFECT_CHANNEL;
            loops = -1;  // Loop forever
            break;

        case AudioType::Effect:
        default:
            // Effects: Play on any free channel (5-15), no loop
            channel = -1;  // Let SDL pick any free channel >= 5
            loops = 0;     // Play once
            break;
    }

    // Play the sound
    int playingChannel = Mix_PlayChannel(channel, chunk, loops);
    if (playingChannel < 0) {
        SDL_Log("ERROR: Failed to play sound '%s': %s",
                to_string(filename).c_str(), Mix_GetError());
        return false;
    }

    // Set volume
    Mix_Volume(playingChannel, static_cast<int>(m_soundVolume * MIX_MAX_VOLUME));

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

int Audio::GetChannelForType(AudioType type) const {
    switch (type) {
        case AudioType::Music:
            // Music uses Mix_Music*, not a channel
            return -1;

        case AudioType::Tune:
            // Find first free channel in tune range (1-4)
            for (int ch = FIRST_TUNE_CHANNEL; ch <= LAST_TUNE_CHANNEL; ++ch) {
                if (!Mix_Playing(ch)) {
                    return ch;
                }
            }
            // All tune channels busy, use first tune channel (will interrupt oldest)
            return FIRST_TUNE_CHANNEL;

        case AudioType::LoopingEffect:
            // Always use dedicated looping effect channel
            return LOOPING_EFFECT_CHANNEL;

        case AudioType::Effect:
            // Use any free channel in effect range (5-15), or let SDL pick
            return -1;  // -1 means "any free channel >= FIRST_EFFECT_CHANNEL"

        default:
            return -1;
    }
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

    switch (type) {
        case AudioType::Music:
            // Stop music (Mix_Music*)
            Mix_HaltMusic();
            m_musicPlaying = false;
            break;

        case AudioType::Tune:
            // Stop all tune channels (1-4)
            for (int ch = FIRST_TUNE_CHANNEL; ch <= LAST_TUNE_CHANNEL; ++ch) {
                Mix_HaltChannel(ch);
            }
            break;

        case AudioType::LoopingEffect:
            // Stop looping effect channel (0)
            Mix_HaltChannel(LOOPING_EFFECT_CHANNEL);
            break;

        case AudioType::Effect:
            // Don't stop one-shot effects - let them finish naturally
            // If we really need to stop them:
            // for (int ch = FIRST_EFFECT_CHANNEL; ch <= LAST_EFFECT_CHANNEL; ++ch) {
            //     Mix_HaltChannel(ch);
            // }
            break;
    }
}

void Audio::Stop() {
    if (!m_initialized) return;

    Mix_HaltMusic();
    Mix_HaltChannel(-1);
    m_musicPlaying = false;
}

const char* Audio::GetSoundPackName(SoundPack pack) const {
    switch (pack) {
        case SoundPack::Amiga:    return "Commodore Amiga";
        case SoundPack::C64:      return "Commodore 64";
        case SoundPack::BBC:      return "BBC Micro";
        case SoundPack::Spectrum: return "Sinclair ZX Spectrum";
        default:                  return "Unknown";
    }
}

void Audio::SetSoundPack(SoundPack pack) {
    if (!m_initialized) return;

    // Don't reload if already using this pack
    if (pack == m_currentPack) {
        return;
    }

    // Update current pack
    m_currentPack = pack;

    // Update sounds directory based on pack
    m_soundsDir = std::string("sounds/") + GetSoundPackName(pack);

    // Clear all cached sounds
    for (auto& [name, chunk] : m_sounds) {
        if (chunk) {
            Mix_FreeChunk(chunk);
        }
    }
    m_sounds.clear();

    SDL_Log("Sound pack changed to: %s", GetSoundPackName(pack));

    // Sounds will be lazy-loaded on next Play() call
}
