#include <string>
#include <windows.h>
#include <random>

namespace STRING_UTILS
{
	inline uint32_t WstringByteSize(const std::wstring& str)
	{
		return static_cast<uint32_t>(str.size() * sizeof(wchar_t));
	}

	inline std::string WstrToStr(const std::wstring& wstr, UINT codePage /*= CP_UTF8*/)
	{
		// 바꿀게 없다면
		if (wstr.empty())
		{
			return std::string();
		}

		// 변환할 문자열 길이 계산
		int needSize = WideCharToMultiByte(
			codePage
			, 0
			, wstr.c_str()
			, static_cast<int>(wstr.size())
			, nullptr
			, 0
			, nullptr
			, nullptr
		);

		std::string result(needSize, 0);

		// 실제 변환
		WideCharToMultiByte(
			codePage
			, 0
			, wstr.c_str()
			, static_cast<int>(wstr.size())
			, &result[0]
			, needSize
			, nullptr
			, nullptr
		);

		return result;
	}

	inline std::string GenerateRandomString(size_t _length) 
	{
		const std::string charset =
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";

		std::random_device rd;

		std::mt19937 generator(rd());

		std::uniform_int_distribution<int> distribution(0, charset.length() - 1);

		std::string random_string;
		random_string.reserve(_length);

		for (size_t i = 0; i < _length; ++i) 
		{
			random_string += charset[distribution(generator)];
		}

		return random_string;
	}
}
