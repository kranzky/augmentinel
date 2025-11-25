#include "Platform.h"
#include "Settings.h"

#define SI_NO_CONVERSION
#include "SimpleIni.h"

#include <fstream>

// Global settings instance
static CSimpleIniA g_ini;
static bool g_initialized = false;

std::wstring settings_path;

// Helper: Convert wstring to UTF-8 string for SimpleIni
static std::string ToUtf8(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    std::string result;
    result.reserve(wstr.size());
    for (wchar_t wc : wstr) {
        if (wc < 0x80) {
            result.push_back(static_cast<char>(wc));
        } else if (wc < 0x800) {
            result.push_back(static_cast<char>(0xC0 | (wc >> 6)));
            result.push_back(static_cast<char>(0x80 | (wc & 0x3F)));
        } else {
            result.push_back(static_cast<char>(0xE0 | (wc >> 12)));
            result.push_back(static_cast<char>(0x80 | ((wc >> 6) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | (wc & 0x3F)));
        }
    }
    return result;
}

// Helper: Convert UTF-8 string to wstring
static std::wstring FromUtf8(const std::string& str) {
    if (str.empty()) return L"";
    std::wstring result;
    result.reserve(str.size());
    size_t i = 0;
    while (i < str.size()) {
        unsigned char c = str[i];
        if (c < 0x80) {
            result.push_back(static_cast<wchar_t>(c));
            i++;
        } else if ((c & 0xE0) == 0xC0) {
            if (i + 1 < str.size()) {
                wchar_t wc = ((c & 0x1F) << 6) | (str[i + 1] & 0x3F);
                result.push_back(wc);
            }
            i += 2;
        } else if ((c & 0xF0) == 0xE0) {
            if (i + 2 < str.size()) {
                wchar_t wc = ((c & 0x0F) << 12) | ((str[i + 1] & 0x3F) << 6) | (str[i + 2] & 0x3F);
                result.push_back(wc);
            }
            i += 3;
        } else {
            i++;
        }
    }
    return result;
}

// Save settings to file
static void SaveSettings() {
    if (!g_initialized) return;
    std::string path = ToUtf8(settings_path);
    SI_Error rc = g_ini.SaveFile(path.c_str());
    if (rc < 0) {
        SDL_Log("Settings: Failed to save to %s (error %d)", path.c_str(), rc);
    }
}

void InitSettings(const std::string& app_name) {
    if (g_initialized) return;

    // Settings path should already be set by Application before this call
    if (settings_path.empty()) {
        SDL_Log("Settings: Warning - settings_path not set, using default");
        settings_path = L"settings.ini";
    }

    std::string path = ToUtf8(settings_path);

    // Configure SimpleIni
    g_ini.SetUnicode(true);
    g_ini.SetMultiKey(false);
    g_ini.SetMultiLine(false);

    // Try to load existing settings file
    SI_Error rc = g_ini.LoadFile(path.c_str());
    if (rc < 0) {
        // File doesn't exist or can't be read - that's OK, we'll create it on first save
        SDL_Log("Settings: No existing settings file at %s, will create on first save", path.c_str());
    } else {
        SDL_Log("Settings: Loaded settings from %s", path.c_str());
    }

    g_initialized = true;
}

std::vector<std::wstring> GetSettingKeys(const std::wstring& section) {
    std::vector<std::wstring> keys;
    if (!g_initialized) return keys;

    std::string sec = ToUtf8(section);

    CSimpleIniA::TNamesDepend keyList;
    g_ini.GetAllKeys(sec.c_str(), keyList);

    for (const auto& entry : keyList) {
        keys.push_back(FromUtf8(entry.pItem));
    }

    return keys;
}

std::wstring GetSetting(const std::wstring& key, const std::wstring& default_value, const std::wstring& section) {
    if (!g_initialized) return default_value;

    std::string sec = ToUtf8(section);
    std::string k = ToUtf8(key);
    std::string def = ToUtf8(default_value);

    const char* value = g_ini.GetValue(sec.c_str(), k.c_str(), def.c_str());
    return FromUtf8(value);
}

int GetSetting(const std::wstring& key, int default_value, const std::wstring& section) {
    if (!g_initialized) return default_value;

    std::string sec = ToUtf8(section);
    std::string k = ToUtf8(key);

    return static_cast<int>(g_ini.GetLongValue(sec.c_str(), k.c_str(), default_value));
}

bool GetFlag(const std::wstring& key, bool default_value, const std::wstring& section) {
    if (!g_initialized) return default_value;

    std::string sec = ToUtf8(section);
    std::string k = ToUtf8(key);

    return g_ini.GetBoolValue(sec.c_str(), k.c_str(), default_value);
}

void RemoveSetting(const std::wstring& key, const std::wstring& section) {
    if (!g_initialized) return;

    std::string sec = ToUtf8(section);
    std::string k = ToUtf8(key);

    g_ini.Delete(sec.c_str(), k.c_str());
    SaveSettings();
}

// Template specializations for SetSetting
void SetSettingImpl(const std::wstring& key, const std::wstring& value, const std::wstring& section) {
    if (!g_initialized) return;

    std::string sec = ToUtf8(section);
    std::string k = ToUtf8(key);
    std::string v = ToUtf8(value);

    g_ini.SetValue(sec.c_str(), k.c_str(), v.c_str());
    SaveSettings();
}

void SetSettingImpl(const std::wstring& key, int value, const std::wstring& section) {
    if (!g_initialized) return;

    std::string sec = ToUtf8(section);
    std::string k = ToUtf8(key);

    g_ini.SetLongValue(sec.c_str(), k.c_str(), value);
    SaveSettings();
}

void SetSettingImpl(const std::wstring& key, bool value, const std::wstring& section) {
    if (!g_initialized) return;

    std::string sec = ToUtf8(section);
    std::string k = ToUtf8(key);

    g_ini.SetBoolValue(sec.c_str(), k.c_str(), value);
    SaveSettings();
}
