#pragma once
#include "Platform.h"

enum class AudioType
{
    Tune,           // jingles at breaks in play
    Music,          // streamed background music
    LoopingEffect,  // the "being watched" drone
    Effect          // one-shot sound effects
};

enum class SoundPack { Amiga, C64, BBC, Spectrum };

// Sound effects, tunes and music using SDL_mixer. Channel 0 is reserved for the
// looping effect, and tunes and effects have separate channel groups so neither
// can cut the other off.
class Audio
{
public:
    Audio();
    ~Audio();
    Audio(const Audio&) = delete;
    Audio& operator=(const Audio&) = delete;

    void Preload(const std::wstring& filename);
    void Play(const std::wstring& filename, AudioType type = AudioType::Effect);
    void Play(const std::wstring& filename, AudioType type, XMFLOAT3 pos);
    void Stop(AudioType type);
    bool IsPlaying(AudioType type) const;

    // Music plays each track once; SetMusicPlaying(true) returns false once it has
    // finished (or nothing is loaded) so the caller can start the next track.
    bool PlayMusic(const fs::path& path);
    bool SetMusicPlaying(bool play);
    void SetMusicVolume(float volume);

    void PositionListener(XMFLOAT3 pos, XMFLOAT3 dir, XMFLOAT3 up);

    SoundPack GetSoundPack() const { return m_pack; }
    void SetSoundPack(SoundPack pack);
    static const char* SoundPackName(SoundPack pack);
    static SoundPack SoundPackFromName(const std::string& name);

private:
    struct ChunkDeleter { void operator()(Mix_Chunk* chunk) const { Mix_FreeChunk(chunk); } };
    struct MusicDeleter { void operator()(Mix_Music* music) const { Mix_FreeMusic(music); } };

    fs::path SoundPath(const std::wstring& filename) const;
    Mix_Chunk* GetChunk(const std::wstring& filename);
    int PlayChunk(const std::wstring& filename, AudioType type, const XMFLOAT3* pos);
    void SetChannelPosition(int channel, XMFLOAT3 pos) const;

    bool m_initialized{ false };
    SoundPack m_pack{ SoundPack::Amiga };
    std::map<std::wstring, std::unique_ptr<Mix_Chunk, ChunkDeleter>> m_sounds;

    std::unique_ptr<Mix_Music, MusicDeleter> m_music;
    bool m_musicPlaying{ false };
    float m_musicVolume{ 1.0f };

    XMFLOAT3 m_listenerPos{};
    XMFLOAT3 m_listenerDir{ 0.0f, 0.0f, 1.0f };
};
