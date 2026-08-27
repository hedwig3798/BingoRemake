#pragma once

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <openssl/evp.h>

namespace PW
{
	std::wstring HashSHA256(const std::wstring& _str)
	{
		unsigned char hash[EVP_MAX_MD_SIZE];
		unsigned int len = 0;

		EVP_MD_CTX* context = EVP_MD_CTX_new();

		if (nullptr == context)
		{
			return std::wstring();
		}

		EVP_DigestInit_ex(context, EVP_sha256(), nullptr);
		EVP_DigestUpdate(context, _str.c_str(), _str.length());
		EVP_DigestFinal_ex(context, hash, &len);
		EVP_MD_CTX_free(context);

		std::wstringstream wss;
		for (unsigned int i = 0; i < len; ++i) 
		{
			wss << std::hex << std::setw(2) << std::setfill(L'0') << (int)hash[i];
		}

		return wss.str();
	}
}