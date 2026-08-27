
set(
    SOURCES
        Login/loginMain.cpp
        Login/LoginProcessor.cpp
)

set(
    HEADERS
        Login/loginMain.h 
        Login/LoginProcessor.h
)


set(ALL_SOURCES ${SOURCES} ${HEADERS})

source_group(
    TREE 
        ${CMAKE_CURRENT_SOURCE_DIR} 
    PREFIX 
        "Source Files" 
    FILES 
        ${ALL_SOURCES}
)

source_group("Packet" FILES ${NETWORK})
source_group("Server Common" FILES ${SERVER_COMMON_HEADER} ${SERVER_COMMON_SOURCE})
source_group("Lua" FILES ${LUA_HELPER_HEADER} ${LUA_HELPER_SOURCE})
add_executable (LoginServer ${ALL_SOURCES} ${NETWORK} ${SERVER_COMMON_HEADER} ${SERVER_COMMON_SOURCE} ${LUA_HELPER_HEADER} ${LUA_HELPER_SOURCE})

target_include_directories(
    LoginServer 
        PRIVATE 
            ${HEADERS}
            ${CMAKE_SOURCE_DIR}/Lua
            ${CMAKE_SOURCE_DIR}/ServerCommon
            ${CMAKE_SOURCE_DIR}/Packet
            ${CMAKE_SOURCE_DIR}/Utils
            ${LUA_INCLUDE_DIR}
            ${LUABRIDGE3_INCLUDE_DIRS}
)

find_package(Boost REQUIRED COMPONENTS asio)

target_link_libraries(
    LoginServer 
        PRIVATE 
            Boost::pfr
            Boost::asio
            PostgreSQL::PostgreSQL
            ${LUA_LIBRARIES}
)


target_compile_definitions(
    LoginServer
        PRIVATE
            SETTING_FILE_PATH="${CMAKE_SOURCE_DIR}/LoginServerValue.lua"
)

