#include "Platform.h"
#include "Settings.h"
#define SI_NO_CONVERSION
#include "SimpleIni.h"

static CSimpleIniA ini{ true /*UTF-8*/, false /*multi-key*/, false /*multi-line*/ };
static fs::path settings_path;

// File I/O goes through std::filesystem rather than SimpleIni's fopen, so paths
// with non-ASCII characters also work on Windows.
static void SaveSettings()
{
    if (settings_path.empty())
        return;

    std::string data;
    ini.Save(data);

    std::ofstream file(settings_path, std::ios::binary | std::ios::trunc);
    if (!file.write(data.data(), static_cast<std::streamsize>(data.size())))
        SDL_Log("Failed to save settings to %s", settings_path.u8string().c_str());
}

void InitSettings(const fs::path& path)
{
    settings_path = path;

    std::ifstream file(path, std::ios::binary);
    if (file)
    {
        std::string data{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
        ini.LoadData(data);
    }

    SDL_Log("Settings: %s", path.u8string().c_str());
}

std::vector<std::wstring> GetSettingKeys(const std::wstring& section)
{
    CSimpleIniA::TNamesDepend names;
    ini.GetAllKeys(to_string(section).c_str(), names);
    names.sort(CSimpleIniA::Entry::LoadOrder());

    std::vector<std::wstring> keys;
    for (const auto& name : names)
        keys.push_back(to_wstring(name.pItem));

    return keys;
}

std::wstring GetSetting(const std::wstring& key, const std::wstring& default_value, const std::wstring& section)
{
    auto value = ini.GetValue(to_string(section).c_str(), to_string(key).c_str(), nullptr);
    return (value && *value) ? to_wstring(value) : default_value;
}

int GetSetting(const std::wstring& key, int default_value, const std::wstring& section)
{
    return static_cast<int>(ini.GetLongValue(to_string(section).c_str(), to_string(key).c_str(), default_value));
}

bool GetFlag(const std::wstring& key, bool default_value, const std::wstring& section)
{
    return ini.GetBoolValue(to_string(section).c_str(), to_string(key).c_str(), default_value);
}

void SetSetting(const std::wstring& key, const std::wstring& value, const std::wstring& section)
{
    ini.SetValue(to_string(section).c_str(), to_string(key).c_str(), to_string(value).c_str());
    SaveSettings();
}

void SetSetting(const std::wstring& key, int value, const std::wstring& section)
{
    ini.SetLongValue(to_string(section).c_str(), to_string(key).c_str(), value);
    SaveSettings();
}

void SetSetting(const std::wstring& key, bool value, const std::wstring& section)
{
    ini.SetBoolValue(to_string(section).c_str(), to_string(key).c_str(), value);
    SaveSettings();
}

void RemoveSetting(const std::wstring& key, const std::wstring& section)
{
    if (ini.Delete(to_string(section).c_str(), to_string(key).c_str()))
        SaveSettings();
}
