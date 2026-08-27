#pragma once

enum class NET_ERROR
{
	NET_OK = 0,			// 정상
	NO_ID_EXITS = 1,	// ID가 존재하지 않음
	PW_NOT_MATCH = 2,	// PW 가 맞지 않음
	UNKNOWN_ERROR,		// 알 수 없는 에러
	END,
};