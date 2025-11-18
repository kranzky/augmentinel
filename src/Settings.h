#pragma once
#include "Platform.h"

// Stub Settings for Phase 1
// Will be properly implemented with SimpleIni in Phase 4

static constexpr auto DEFAULT_SECTION = L"Main";
extern std::wstring settings_path;

inline void InitSettings(const std::string& app_name) {
    SDL_Log("Settings stub initialized");
}

inline std::vector<std::wstring> GetSettingKeys(const std::wstring& section = DEFAULT_SECTION) {
    return {};
}

inline std::wstring GetSetting(const std::wstring& key, const std::wstring& default_value, const std::wstring& section = DEFAULT_SECTION) {
    return default_value;
}

inline int GetSetting(const std::wstring& key, int default_value, const std::wstring& section = DEFAULT_SECTION) {
    return default_value;
}

inline bool GetFlag(const std::wstring& key, bool default_value, const std::wstring& section = DEFAULT_SECTION) {
    return default_value;
}

inline void RemoveSetting(const std::wstring& key, const std::wstring& section = DEFAULT_SECTION) {
}

template<typename T>
void SetSetting(const std::wstring& key, const T& value, const std::wstring& section = DEFAULT_SECTION) {
    // Stub
}
