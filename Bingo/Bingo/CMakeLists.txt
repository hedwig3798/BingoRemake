add_executable (Bingo "Bingo.cpp" "Bingo.h")

find_package(OpenSSL REQUIRED)

find_package(PostgreSQL REQUIRED)

# 라이브러리 연결
target_link_libraries(
    Bingo 
    PRIVATE 
        OpenSSL::Crypto 
        PostgreSQL::PostgreSQL
)

INCLUDE_DIRECTORIES(../Hash)