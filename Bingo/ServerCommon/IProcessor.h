#pragma once

#include <vector>
#include <memory>

class Session;

class IProcessor
{
public:
	IProcessor() = default;
	virtual ~IProcessor() {};

public:
	virtual bool Process() = 0;
	virtual void AddMsg(std::shared_ptr<Session> _session, std::vector<char>&& _buffer) = 0;
	virtual void Init() = 0;
};