#pragma once

enum class NET_ERROR
{
	NET_OK = 0,				// 정상
	NO_ID_EXITS = 1,		// ID가 존재하지 않음
	PW_NOT_MATCH = 2,		// PW 가 맞지 않음
	DB_QUERY_ERROR = 3,		// DB 에서 데이터를 가져오거나 참조하는데 문제 발생
	DB_SEND_ERROR = 4,		// DB 에 쿼리를 보내는것에 실패함
	ID_EXITS = 5,			// ID가 이미 있음
	UNKNOWN_ERROR,			// 알 수 없는 에러
	END,
};