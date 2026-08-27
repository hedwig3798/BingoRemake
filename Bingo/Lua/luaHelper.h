#pragma once

#include <lua.hpp>
#include <luabridge3/LuaBridge/LuaBridge.h>
#include <iostream>

class LuaHelper
{
private:
	lua_State* L;

public:
	LuaHelper();
	~LuaHelper();

public:
	void InitLua();

	void ReadScript(const std::string& _path);

	/// <summary>
	/// 기본 타입에 한한 변수 반환
	/// </summary>
	/// <typeparam name="T">타입</typeparam>
	/// <param name="_name">변수 이름</param>
	/// <returns>리턴 값</returns>
	template <typename T>
	T Get(const std::string& _name)
	{
		luabridge::LuaRef ref = luabridge::getGlobal(L, _name.c_str());
		auto val = ref.cast<T>();

		if (val)
		{
			return val.value();
		}

		std::cerr << "cannot find " << _name << " in lua global spcae\n";
		return T();
	}
};