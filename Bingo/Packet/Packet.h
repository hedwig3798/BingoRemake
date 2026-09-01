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
	NET_ERROR m_netError = NET_ERROR::NET_OK;

// ========================================================================

#pragma pack(push, 1)
struct PacketHeader
{
	uint16_t m_size;
	uint32_t m_ID;
};

// ========================================================================
///
/// 패킷 명명 규칙
/// XTX : X 에서 X 로 보내는 패킷 / 단, A 는 모든 곳
/// RES : 요청, 답 패킷이 오기를 기대하는 패킷
/// ACK : RES 패킷에 대한 답
/// NONE : 단순히 보내기만 하는 패킷
///
struct LTD_RES_ACCESS
{
	DECLARE_PACKET(LTD_RES_ACCESS)
};

struct DTA_ACK_ACCESS
{
	DECLARE_PACKET(DTA_ACK_ACCESS)
};

struct GTD_RES_ACCESS
{
	DECLARE_PACKET(GTD_RES_ACCESS)
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

struct CTL_RES_ID_AVAILABLITY
{
	DECLARE_PACKET(CTL_RES_ID_AVAILABLITY)
	std::string m_ID;
};

struct LTC_ACK_ID_AVAILABLITY
{
	DECLARE_PACKET(LTC_ACK_ID_AVAILABLITY)
	bool m_isAvailablity;
};

#pragma pack(pop)