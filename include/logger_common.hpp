#pragma once

#include <cstdlib>

namespace ricox {
constexpr size_t LOG_QUEUE_SIZE = 8 * 1024 * 1024;	// 8 MB default size for log queue

enum class log_type : int8_t {
	CHAR = 0,
	INTEGER = 1,
	LONG_INTEGER = 2,
	LONG_LONG_INTEGER = 3,
	UNSIGNED_INTEGER = 4,
	UNSIGNED_LONG_INTEGER = 5,
	UNSIGNED_LONG_LONG_INTEGER = 6,
	FLOAT = 7,
	DOUBLE = 8
};

struct log_entry {
	log_type type;

	union {
		char c;
		int i;
		long l;
		long long ll;
		unsigned int ui;
		unsigned long ul;
		unsigned long long ull;
		float f;
		double d;
	} value;
};

}  // namespace ricox