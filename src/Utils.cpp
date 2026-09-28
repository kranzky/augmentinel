#include "Platform.h"
#include "Utils.h"

#ifdef PLATFORM_WINDOWS
void Fail(int hr, const wchar_t* pszOperation)
{
	if (FAILED(hr))
	{
		char sz[256];
		wsprintfA(sz, "%S failed with %08lx", pszOperation, hr);
		ShowWindow(Application::Hwnd(), SW_HIDE);
#ifdef _DEBUG
		__debugbreak();
#endif
		throw std::exception(sz);
	}
}

std::wstring WindowText(HWND hwnd)
{
	auto len = GetWindowTextLength(hwnd) + 1;	// include null terminator
	std::vector<wchar_t> str(len);
	GetWindowText(hwnd, str.data(), static_cast<int>(str.size()));
	return str.data();
}

fs::path ModulePath(HMODULE hmod)
{
	wchar_t wpath[MAX_PATH];
	GetModuleFileName(hmod, wpath, _countof(wpath));
	return wpath;
}

fs::path ModuleDirectory(HMODULE hmod)
{
	return ModulePath(hmod).remove_filename();
}

fs::path WorkingDirectory()
{
	wchar_t wpath[MAX_PATH];
	GetCurrentDirectory(_countof(wpath), wpath);
	return wpath;
}

std::vector<uint8_t> FileContents(const std::wstring& filename)
{
	wchar_t szPath[MAX_PATH];
	GetModuleFileName(NULL, szPath, _countof(szPath));

	// Form a full path relative to the module path.
	std::wstring full_path = szPath;
	full_path = full_path.substr(0, full_path.rfind(L'\\') + 1);
	full_path += filename;

	// Try module relative, falling back to CWD relative path.
	HANDLE hfile = CreateFile(full_path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hfile == INVALID_HANDLE_VALUE)
		hfile = CreateFile(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

	if (hfile == INVALID_HANDLE_VALUE)
	{
		auto str = "File not found: " + to_string(filename);
		throw std::runtime_error(str);
	}

	DWORD dwSize = GetFileSize(hfile, NULL), dwRead;
	std::vector<BYTE> file(dwSize);
	if (!ReadFile(hfile, file.data(), dwSize, &dwRead, NULL))
		file.clear();
	CloseHandle(hfile);

	return file;
}
#else
std::vector<uint8_t> FileContents(const fs::path& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
        throw std::runtime_error("File not found: " + path.u8string());

    return { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
}
#endif

std::wstring to_wstring(const std::string& str)
{
    std::wstring result;
    result.reserve(str.size());

    for (size_t i = 0; i < str.size();)
    {
        auto c = static_cast<unsigned char>(str[i]);
        auto length = (c < 0x80) ? 1 : ((c & 0xe0) == 0xc0) ? 2 : ((c & 0xf0) == 0xe0) ? 3 : ((c & 0xf8) == 0xf0) ? 4 : 0;
        if (!length || i + length > str.size())
        {
            result.push_back(L'\ufffd');   // invalid or truncated sequence
            ++i;
            continue;
        }

        uint32_t code_point = (length == 1) ? c : (c & (0x7f >> length));
        for (auto n = 1; n < length; ++n)
            code_point = (code_point << 6) | (static_cast<unsigned char>(str[i + n]) & 0x3f);
        i += length;

        if constexpr (sizeof(wchar_t) == 2)
        {
            if (code_point >= 0x10000)
            {
                code_point -= 0x10000;
                result.push_back(static_cast<wchar_t>(0xd800 + (code_point >> 10)));
                result.push_back(static_cast<wchar_t>(0xdc00 + (code_point & 0x3ff)));
                continue;
            }
        }
        result.push_back(static_cast<wchar_t>(code_point));
    }

    return result;
}

std::string to_string(const std::wstring& wstr)
{
    std::string result;
    result.reserve(wstr.size());

    for (size_t i = 0; i < wstr.size(); ++i)
    {
        auto code_point = static_cast<uint32_t>(wstr[i]);

        // Combine UTF-16 surrogate pairs.
        if (code_point >= 0xd800 && code_point <= 0xdbff && i + 1 < wstr.size())
        {
            auto low = static_cast<uint32_t>(wstr[i + 1]);
            if (low >= 0xdc00 && low <= 0xdfff)
            {
                code_point = 0x10000 + ((code_point - 0xd800) << 10) + (low - 0xdc00);
                ++i;
            }
        }

        if (code_point < 0x80)
        {
            result.push_back(static_cast<char>(code_point));
        }
        else if (code_point < 0x800)
        {
            result.push_back(static_cast<char>(0xc0 | (code_point >> 6)));
            result.push_back(static_cast<char>(0x80 | (code_point & 0x3f)));
        }
        else if (code_point < 0x10000)
        {
            result.push_back(static_cast<char>(0xe0 | (code_point >> 12)));
            result.push_back(static_cast<char>(0x80 | ((code_point >> 6) & 0x3f)));
            result.push_back(static_cast<char>(0x80 | (code_point & 0x3f)));
        }
        else
        {
            result.push_back(static_cast<char>(0xf0 | (code_point >> 18)));
            result.push_back(static_cast<char>(0x80 | ((code_point >> 12) & 0x3f)));
            result.push_back(static_cast<char>(0x80 | ((code_point >> 6) & 0x3f)));
            result.push_back(static_cast<char>(0x80 | (code_point & 0x3f)));
        }
    }

    return result;
}

std::mt19937& random_source()
{
	static std::random_device rd;
	static std::mt19937 rng(rd());
	return rng;
}

uint32_t random_uint32()
{
	return (random_source())();
}
