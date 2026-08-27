#include "luaHelper.h"

LuaHelper::LuaHelper()
	: L(nullptr)
{

}

LuaHelper::~LuaHelper()
{

}

void LuaHelper::InitLua()
{
	if (nullptr != L)
	{
		return;
	}

	L = luaL_newstate();
}

void LuaHelper::ReadScript(const std::string& _path)
{
	if (LUA_OK != luaL_dofile(L, SETTING_FILE_PATH))
	{
		std::cerr << "cannot Load " << _path << " File";
		std::cerr << SETTING_FILE_PATH << "\n";
	}
}
