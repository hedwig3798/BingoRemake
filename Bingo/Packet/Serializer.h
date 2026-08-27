#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <type_traits>
#include <cstring>
#include <stdexcept>
#include <boost/pfr.hpp>
#include <cstdint>
#include "Packet.h"

/// <summary>
/// 패킷 데이터를 쓰는 클래스
/// </summary>
class PacketWriter
{
private:
	std::vector<char> m_buffer;

public:
	/// <summary>
	/// memcopy 가 가능한 타입에 대한 버퍼에 쓰기
	/// </summary>
	/// <typeparam name="T">타입</typeparam>
	/// <param name="_data">데이터</param>
	template <typename T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
	void Write(const T& _data)
	{
		const char* ptr = reinterpret_cast<const char*>(&_data);
		m_buffer.insert(m_buffer.end(), ptr, ptr + sizeof(T));
		AddSize(sizeof(T));
	}

	/// <summary>
	/// string 특수화
	/// </summary>
	/// <param name="_str">데이터</param>
	void Write(const std::string& _str)
	{
		uint16_t len = static_cast<uint16_t>(_str.size());
		Write(len);
		const char* ptr = reinterpret_cast<const char*>(_str.data());
		m_buffer.insert(m_buffer.end(), ptr, ptr + len);

		AddSize(len);
	}

	/// <summary>
	/// vector 특수화
	/// </summary>
	/// <typeparam name="T">데이터</typeparam>
	/// <param name="_vec">벡터</param>
	template <typename T>
	void Write(const std::vector<T>& _vec)
	{
		uint16_t count = static_cast<uint16_t>(_vec.size());
		Write(count);
		for (const auto& item : _vec)
		{
			Write(item);
		}
	}

	/// <summary>
	/// 버퍼 가져오기
	/// </summary>
	/// <returns>버퍼</returns>
	std::vector<char>& GetBuffer() { return m_buffer; }

	void clear() { m_buffer.clear(); };

private:
	void AddSize(uint16_t _size)
	{
		reinterpret_cast<uint16_t*>(m_buffer.data())[0] += _size;
	}
};

/// <summary>
/// 패킷 데이터를 읽는 클래스
/// </summary>
class PacketReader 
{
private:
	std::vector<char> m_buffer;
	size_t m_offset;

	/// <summary>
	/// 패킷 사이즈 체크 함수
	/// </summary>
	/// <param name="_sizeToRead">앞으로 읽을 사이즈</param>
	void CheckSize(size_t _sizeToRead) const 
	{
		if (m_offset + _sizeToRead > m_buffer.size()) 
		{
			throw std::out_of_range("패킷 버퍼 크기 초과 읽기 시도!");
		}
	}

public:
	PacketReader() 
		: m_buffer()
		, m_offset(0) 
	{
	}

public:

	void SetBuffer(std::vector<char>& data, uint16_t _headerSize)
	{
		m_buffer.clear();
		m_buffer.resize(data.size() - _headerSize);

		memmove(m_buffer.data(), data.data() + _headerSize, data.size() - _headerSize);
		m_offset = 0;
	}

	/// <summary>
	/// 복사 가능한 데이터 읽기
	/// </summary>
	/// <typeparam name="T">타입</typeparam>
	/// <param name="_data">데이터</param>
	template <typename T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
	void Read(T& _data) 
	{
		CheckSize(sizeof(T)); 
		std::memcpy(&_data, m_buffer.data() + m_offset, sizeof(T));
		m_offset += sizeof(T);
	}

	/// <summary>
	/// string 특수화
	/// </summary>
	/// <param name="_str">데이터</param>
	void Read(std::string& _str) 
	{
		uint16_t len;
		Read(len); 

		CheckSize(len);
		_str.assign(reinterpret_cast<const char*>(m_buffer.data() + m_offset), len);
		m_offset += len;
	}

	/// <summary>
	/// 벡터 특수화
	/// </summary>
	/// <typeparam name="T">타입</typeparam>
	/// <param name="vec">데이터</param>
	template <typename T>
	void Read(std::vector<T>& vec) 
	{
		uint16_t count;
		Read(count); 

		vec.resize(count);
		for (auto& item : vec) 
		{
			Read(item);
		}
	}
};

/// <summary>
/// 패킷 직렬화
/// </summary>
/// <typeparam name="T">패킷 구조체 타입</typeparam>
/// <param name="_writer">패킷 핼퍼 클래스</param>
/// <param name="_packet">패킷</param>
template <typename T>
void Serialize(PacketWriter& _writer, const T& _packet)
{
	_writer.clear();

	PacketHeader header;
	header.m_size = 0;
	header.m_ID = T::PACKET_ID;

	_writer.Write(header.m_size);
	_writer.Write(header.m_ID);
	boost::pfr::for_each_field(_packet, [&_writer](const auto& field)
		{
			_writer.Write(field);
		});
};

/// <summary>
/// 패킷 역직렬화
/// </summary>
/// <typeparam name="T">패킷 구조체 타입</typeparam>
/// <param name="_writer">패킷 핼퍼 클래스</param>
/// <param name="_packet">패킷</param>
template <typename T>
void Deserialize(PacketReader& _reader, T& _packet) 
{
	boost::pfr::for_each_field(_packet, [&_reader](auto& field) 
		{
			_reader.Read(field);
		});
};
