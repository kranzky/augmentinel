#pragma once
#include "Platform.h"

static constexpr auto DEFAULT_SECTION = L"Main";

// Load settings from an INI file, which is created on the first change.
void InitSettings(const fs::path& path);

std::vector<std::wstring> GetSettingKeys(const std::wstring& section = DEFAULT_SECTION);
std::wstring GetSetting(const std::wstring& key, const std::wstring& default_value, const std::wstring& section = DEFAULT_SECTION);
int GetSetting(const std::wstring& key, int default_value, const std::wstring& section = DEFAULT_SECTION);
bool GetFlag(const std::wstring& key, bool default_value, const std::wstring& section = DEFAULT_SECTION);

// Each change is saved immediately.
void SetSetting(const std::wstring& key, const std::wstring& value, const std::wstring& section = DEFAULT_SECTION);
void SetSetting(const std::wstring& key, int value, const std::wstring& section = DEFAULT_SECTION);
void SetSetting(const std::wstring& key, bool value, const std::wstring& section = DEFAULT_SECTION);
void RemoveSetting(const std::wstring& key, const std::wstring& section = DEFAULT_SECTION);
