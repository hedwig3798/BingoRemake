set(CMAKE_MFC_FLAG 2)

set(
    SOURCES
        clientMain.cpp 
        Login/LoginApp.cpp 
        Login/LoginDialog.cpp 

        NetLayer/NetLayer.cpp 
)

set(
    HEADERS
        clientMain.h 
        Login/LoginApp.h 
        Login/LoginDialog.h 

        NetLayer/NetLayer.h 
)

set(
    RESOURCES
        Login/resource.h
        Login/Login.rc
)

set(ALL_SOURCES ${SOURCES} ${HEADERS} ${RESOURCES})
source_group(
    TREE 
        ${CMAKE_CURRENT_SOURCE_DIR} 
    PREFIX 
        "Source Files" 
    FILES 
        ${ALL_SOURCES}
)

source_group("Packet" FILES ${NETWORK})
add_executable (Client WIN32 ${ALL_SOURCES} ${NETWORK})
target_include_directories(
    Client 
        PRIVATE 
            ${HEADERS}
            ${CMAKE_SOURCE_DIR}/Packet 
            ${CMAKE_SOURCE_DIR}/Lua
            ${CMAKE_SOURCE_DIR}/Utils
            ${LUA_INCLUDE_DIR}
            ${LUABRIDGE3_INCLUDE_DIRS}
)

target_link_libraries(
    Client 
        PRIVATE 
            Boost::pfr 
            ${LUA_LIBRARIES}
            OpenSSL::Crypto 
)

