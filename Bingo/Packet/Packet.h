#pragma once

#include <string>

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
    static constexpr uint32_t PACKET_ID = HashPacketName(#PacketName);

// ========================================================================

#pragma pack(push, 1)
struct PacketHeader
{
	uint16_t m_size;
	uint32_t m_ID;
};

// ========================================================================

struct LOGIN_PACKET_SEND
{
	DECLARE_PACKET(LOGIN_PACKET_SEND)
	std::string m_ID;
	std::string m_hashedPW;
};

struct LOGIN_PACKET_RECV
{
	DECLARE_PACKET(LOGIN_PACKET_RECV)
	bool m_isSuccess;
	std::string m_failReason;
};

#pragma pack(pop)