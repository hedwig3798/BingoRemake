#pragma once

#include <string>
#include "NetError.h"

constexpr uint32_t HashPacketName(const char* str)
{
	uint32_t hash = 2166136261u;
	for (size_t i = 0; str[i] != '\0'; ++i)
	{
		hash ^= static_cast<uint32_t>(str[i]);
		hash *= 16777619u;
	}
	return hash;
}

#define DECLARE_PACKET(PacketName) \
    static constexpr uint32_t PACKET_ID = HashPacketName(#PacketName); \
	NET_ERROR m_netError;

// ========================================================================

#pragma pack(push, 1)
struct PacketHeader
{
	uint16_t m_size;
	uint32_t m_ID;
};

// ========================================================================

struct LTD_ACCESS
{
	DECLARE_PACKET(LTD_ACCESS)
};

struct DTL_ACCESS_SUCCESS
{
	DECLARE_PACKET(DTL_ACCESS_SUCCESS)
};

struct GTD_ACCESS
{
	DECLARE_PACKET(GTD_ACCESS)
};

struct CTL_RES_LOGIN
{
	DECLARE_PACKET(CTL_RES_LOGIN)
	std::string m_ID;
	std::string m_hashedPW;
};

struct LTC_ACK_LOGIN
{
	DECLARE_PACKET(LTC_ACK_LOGIN)
	bool m_isSuccess;
};

struct LTD_RES_LOGIN_DATA
{
	DECLARE_PACKET(LTD_RES_LOGIN_DATA)
	std::string m_ID;
	std::string m_hashedPW;
};

struct DTL_ACK_LOGIN_DATA
{
	DECLARE_PACKET(DTL_ACK_LOGIN_DATA)
	std::string m_ID;
	std::string m_hashedPW;
	std::string m_saltedPW;
	std::string m_salt;
};

#pragma pack(pop)