#include "Platform.h"
#include "Audio.h"

constexpr auto NUM_CHANNELS = 16;
constexpr auto LOOPING_EFFECT_CHANNEL = 0;  // reserved, so never picked automatically
constexpr auto FIRST_TUNE_CHANNEL = 1;
constexpr auto LAST_TUNE_CHANNEL = 4;
constexpr auto FIRST_EFFECT_CHANNEL = 5;
constexpr auto LAST_EFFECT_CHANNEL = NUM_CHANNELS - 1;
constexpr auto TUNE_GROUP = 1;
constexpr auto EFFECT_GROUP = 2;

// Sounds are inaudible beyond this distance (roughly the fog distance), and at full
// volume within the minimum distance.
constexpr auto MIN_SOUND_DISTANCE = 1.0f;
constexpr auto MAX_SOUND_DISTANCE = 32.0f;

Audio::Audio()
{
    if (!SDL_WasInit(SDL_INIT_AUDIO) && SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
    {
        SDL_Log("Audio unavailable: %s", SDL_GetError());
        return;
    }

    if (!(Mix_Init(MIX_INIT_MP3) & MIX_INIT_MP3))
        SDL_Log("MP3 support unavailable, so no music: %s", Mix_GetError());

    if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024) < 0)
    {
        SDL_Log("Failed to open audio device: %s", Mix_GetError());
        return;
    }

    Mix_AllocateChannels(NUM_CHANNELS);
    Mix_ReserveChannels(LOOPING_EFFECT_CHANNEL + 1);
    Mix_GroupChannels(FIRST_TUNE_CHANNEL, LAST_TUNE_CHANNEL, TUNE_GROUP);
    Mix_GroupChannels(FIRST_EFFECT_CHANNEL, LAST_EFFECT_CHANNEL, EFFECT_GROUP);

    m_initialized = true;
}

Audio::~Audio()
{
    if (!m_initialized)
        return;

    // Halt playback before the chunks and music it uses are freed.
    Mix_HaltChannel(-1);
    Mix_HaltMusic();
    m_sounds.clear();
    m_music.reset();

    Mix_CloseAudio();
    Mix_Quit();
}

////////////////////////////////////////////////////////////////////////////////
// Sound packs

const char* Audio::SoundPackName(SoundPack pack)
{
    switch (pack)
    {
    case SoundPack::Amiga:    return "Commodore Amiga";
    case SoundPack::C64:      return "Commodore 64";
    case SoundPack::BBC:      return "BBC Micro";
    case SoundPack::Spectrum: return "Sinclair ZX Spectrum";
    }
    return "Commodore Amiga";
}

SoundPack Audio::SoundPackFromName(const std::string& name)
{
    for (auto pack : { SoundPack::Amiga, SoundPack::C64, SoundPack::BBC, SoundPack::Spectrum })
    {
        if (name == SoundPackName(pack))
            return pack;
    }
    return SoundPack::Amiga;
}

void Audio::SetSoundPack(SoundPack pack)
{
    if (pack == m_pack)
        return;

    m_pack = pack;
    SDL_Log("Sound pack: %s", SoundPackName(pack));

    // Reload everything that was loaded from the old pack, so nothing loads mid-game.
    // Freeing a chunk halts any channel still playing it.
    std::vector<std::wstring> names;
    for (auto& [name, chunk] : m_sounds)
        names.push_back(name);

    m_sounds.clear();
    for (auto& name : names)
        Preload(name);
}

fs::path Audio::SoundPath(const std::wstring& filename) const
{
    return g_resourcePath / "sounds" / SoundPackName(m_pack) / filename;
}

////////////////////////////////////////////////////////////////////////////////
// Effects and tunes

void Audio::Preload(const std::wstring& filename)
{
    GetChunk(filename);
}

Mix_Chunk* Audio::GetChunk(const std::wstring& filename)
{
    if (!m_initialized)
        return nullptr;

    auto it = m_sounds.find(filename);
    if (it != m_sounds.end())
        return it->second.get();

    auto path = SoundPath(filename);
    auto chunk = Mix_LoadWAV(path.u8string().c_str());
    if (!chunk)
        SDL_Log("Failed to load sound %s: %s", path.u8string().c_str(), Mix_GetError());

    // Failures are cached too, so a missing file is only reported once.
    m_sounds[filename].reset(chunk);
    return chunk;
}

void Audio::Play(const std::wstring& filename, AudioType type)
{
    PlayChunk(filename, type, nullptr);
}

void Audio::Play(const std::wstring& filename, AudioType type, XMFLOAT3 pos)
{
    PlayChunk(filename, type, &pos);
}

int Audio::PlayChunk(const std::wstring& filename, AudioType type, const XMFLOAT3* pos)
{
    assert(type != AudioType::Music);

    auto chunk = GetChunk(filename);
    if (!chunk)
        return -1;

    auto channel = LOOPING_EFFECT_CHANNEL;
    if (type != AudioType::LoopingEffect)
    {
        // Use a free channel from the group, or cut off the oldest sound in it.
        auto group = (type == AudioType::Tune) ? TUNE_GROUP : EFFECT_GROUP;
        channel = Mix_GroupAvailable(group);
        if (channel < 0)
            channel = Mix_GroupOldest(group);
    }

    // Position before playing. Unpositioned sounds clear any position left on the
    // channel by an earlier sound, which would otherwise still pan and attenuate it.
    if (pos)
        SetChannelPosition(channel, *pos);
    else
        Mix_SetPosition(channel, 0, 0);

    auto loops = (type == AudioType::LoopingEffect) ? -1 : 0;
    if (Mix_PlayChannel(channel, chunk, loops) < 0)
    {
        SDL_Log("Failed to play sound: %s", Mix_GetError());
        return -1;
    }

    return channel;
}

void Audio::Stop(AudioType type)
{
    if (!m_initialized)
        return;

    switch (type)
    {
    case AudioType::Music:         SetMusicPlaying(false); break;
    case AudioType::Tune:          Mix_HaltGroup(TUNE_GROUP); break;
    case AudioType::LoopingEffect: Mix_HaltChannel(LOOPING_EFFECT_CHANNEL); break;
    case AudioType::Effect:        Mix_HaltGroup(EFFECT_GROUP); break;
    }
}

bool Audio::IsPlaying(AudioType type) const
{
    if (!m_initialized)
        return false;

    switch (type)
    {
    case AudioType::Music:         return m_musicPlaying && Mix_PlayingMusic();
    case AudioType::Tune:          return Mix_GroupNewer(TUNE_GROUP) >= 0;
    case AudioType::LoopingEffect: return Mix_Playing(LOOPING_EFFECT_CHANNEL) != 0;
    case AudioType::Effect:        return Mix_GroupNewer(EFFECT_GROUP) >= 0;
    }
    return false;
}

void Audio::PositionListener(XMFLOAT3 pos, XMFLOAT3 dir, XMFLOAT3 /*up*/)
{
    m_listenerPos = pos;
    m_listenerDir = dir;
}

// Stereo panning and distance attenuation in the horizontal plane.
void Audio::SetChannelPosition(int channel, XMFLOAT3 pos) const
{
    auto dx = pos.x - m_listenerPos.x;
    auto dy = pos.y - m_listenerPos.y;
    auto dz = pos.z - m_listenerPos.z;
    auto distance = std::sqrt(dx * dx + dy * dy + dz * dz);

    auto attenuation = std::clamp((distance - MIN_SOUND_DISTANCE) / (MAX_SOUND_DISTANCE - MIN_SOUND_DISTANCE), 0.0f, 1.0f);

    // SDL_mixer angles are clockwise degrees from straight ahead.
    auto relative_angle = std::atan2(dx, dz) - std::atan2(m_listenerDir.x, m_listenerDir.z);
    auto degrees = static_cast<int>(std::lround(XMConvertToDegrees(relative_angle))) % 360;
    if (degrees < 0)
        degrees += 360;

    // Distance 0 is loudest; 255 is quiet but not silent.
    Mix_SetPosition(channel, static_cast<Sint16>(degrees), static_cast<Uint8>(attenuation * 255.0f));
}

////////////////////////////////////////////////////////////////////////////////
// Music

bool Audio::PlayMusic(const fs::path& path)
{
    if (!m_initialized)
        return false;

    Mix_HaltMusic();
    m_music.reset(Mix_LoadMUS(path.u8string().c_str()));
    m_musicPlaying = false;

    if (!m_music || Mix_PlayMusic(m_music.get(), 0) < 0)
    {
        SDL_Log("Failed to play music %s: %s", path.u8string().c_str(), Mix_GetError());
        m_music.reset();
        return false;
    }

    Mix_VolumeMusic(static_cast<int>(m_musicVolume * MIX_MAX_VOLUME));
    m_musicPlaying = true;
    return true;
}

bool Audio::SetMusicPlaying(bool play)
{
    if (!m_initialized || !m_music)
        return false;

    // Music that has finished (rather than been paused) needs a new track.
    if (play && !Mix_PlayingMusic())
        return false;

    if (play != m_musicPlaying)
    {
        if (play)
            Mix_ResumeMusic();
        else
            Mix_PauseMusic();

        m_musicPlaying = play;
    }

    return true;
}

void Audio::SetMusicVolume(float volume)
{
    m_musicVolume = std::clamp(volume, 0.0f, 1.0f);
    if (m_initialized)
        Mix_VolumeMusic(static_cast<int>(m_musicVolume * MIX_MAX_VOLUME));
}
