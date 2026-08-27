cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project ("Bingo")
find_package(boost_pfr CONFIG REQUIRED)
find_package(Lua REQUIRED)
find_path(LUABRIDGE3_INCLUDE_DIRS "luabridge3/LuaBridge/Any.h")
find_package(OpenSSL REQUIRED)

find_package(PostgreSQL REQUIRED)
set(
	NETWORK
		${CMAKE_CURRENT_SOURCE_DIR}/Packet/Packet.h
		${CMAKE_CURRENT_SOURCE_DIR}/Packet/Serializer.h
		${CMAKE_CURRENT_SOURCE_DIR}/Packet/NetError.h
)

set(
	SERVER_COMMON_HEADER
		${CMAKE_CURRENT_SOURCE_DIR}/ServerCommon/Server.h
		${CMAKE_CURRENT_SOURCE_DIR}/ServerCommon/Session.h
		${CMAKE_CURRENT_SOURCE_DIR}/ServerCommon/IProcessor.h
)

set(
	SERVER_COMMON_SOURCE
		${CMAKE_CURRENT_SOURCE_DIR}/ServerCommon/Server.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/ServerCommon/Session.cpp
)

set(
	LUA_HELPER_HEADER
		${CMAKE_CURRENT_SOURCE_DIR}/Lua/luaHelper.h
)

set(
	LUA_HELPER_SOURCE
		${CMAKE_CURRENT_SOURCE_DIR}/Lua/luaHelper.cpp
)

add_subdirectory ("Bingo")
add_subdirectory ("GameServer")
add_subdirectory ("LoginServer")
add_subdirectory ("DBServer")
add_subdirectory ("Client")
