#pragma once
#include "Platform.h"

static constexpr auto DEFAULT_SECTION = L"Main";
extern std::wstring settings_path;

// Initialize settings system - must be called after settings_path is set
void InitSettings(const std::string& app_name);

// Get all keys in a section
std::vector<std::wstring> GetSettingKeys(const std::wstring& section = DEFAULT_SECTION);

// Get string setting with default
std::wstring GetSetting(const std::wstring& key, const std::wstring& default_value, const std::wstring& section = DEFAULT_SECTION);

// Get integer setting with default
int GetSetting(const std::wstring& key, int default_value, const std::wstring& section = DEFAULT_SECTION);

// Get boolean flag with default
bool GetFlag(const std::wstring& key, bool default_value, const std::wstring& section = DEFAULT_SECTION);

// Remove a setting
void RemoveSetting(const std::wstring& key, const std::wstring& section = DEFAULT_SECTION);

// Implementation functions (called by template)
void SetSettingImpl(const std::wstring& key, const std::wstring& value, const std::wstring& section);
void SetSettingImpl(const std::wstring& key, int value, const std::wstring& section);
void SetSettingImpl(const std::wstring& key, bool value, const std::wstring& section);

// Template wrapper that dispatches to implementation
template<typename T>
void SetSetting(const std::wstring& key, const T& value, const std::wstring& section = DEFAULT_SECTION) {
    SetSettingImpl(key, value, section);
}
