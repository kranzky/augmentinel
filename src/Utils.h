#pragma once

#ifdef PLATFORM_WINDOWS
void Fail(HRESULT hr, LPCWSTR pszOperation);
fs::path ModulePath(HMODULE hmod = NULL);
fs::path ModuleDirectory(HMODULE hmod = NULL);
fs::path WorkingDirectory();
std::wstring WindowText(HWND hwnd);
#endif

// Cross-platform file reading
std::vector<uint8_t> FileContents(const std::wstring& filename);


template <typename T>
void aspect_correct(T& current_width, T& current_height, const float aspect_ratio)
{
	auto current_aspect = static_cast<float>(current_width) / current_height;
	if (current_aspect > aspect_ratio)
		current_width = static_cast<T>(current_height * aspect_ratio);
	else
		current_height = static_cast<T>(current_width / aspect_ratio);
}

template<size_t size, typename T>
constexpr auto array_slice(const T& src, const size_t start)
{
	std::array<uint8_t, size> dst;
	std::copy(src.begin() + start, src.begin() + start + size, dst.begin());
	return dst;
}

// Work-around for using make_shared with extended aligned storage sizes.
template <typename T, typename ...Args>
std::shared_ptr<T> make_shared_aligned(Args&& ...args)
{
	return std::shared_ptr<T>(new T(std::forward<Args>(args)...));
}

inline bool operator==(XMFLOAT3& l, XMFLOAT3& r)
{
	return l.x == r.x && l.y == r.y && l.z == r.z;
}

inline bool operator!=(XMFLOAT3& l, XMFLOAT3& r)
{
	return l.x != r.x || l.y != r.y || l.z != r.z;
}

// UTF-8 string conversion (replaces deprecated std::codecvt_utf8)
inline std::wstring to_wstring(const std::string& str)
{
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
			i++; // Skip invalid/4-byte sequences
		}
	}
	return result;
}

inline std::string to_string(const std::wstring& wstr)
{
	if (wstr.empty()) return "";
	std::string result;
	result.reserve(wstr.size() * 3); // Worst case: 3 bytes per wchar
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

inline float pitch_from_dir(const XMFLOAT3& dir)
{
	return std::asin(-dir.y);
}

inline float yaw_from_dir(const XMFLOAT3& dir)
{
	return std::atan2(dir.x, dir.z);
}

std::mt19937& random_source();
uint32_t random_uint32();
