#pragma once
#include <cstdint>
#include <string>
#include <SharedGPU_CPU/SharedTypes.h>

typedef std::string stdstr_t;
typedef const char* cstr_t;
typedef uint8_t byte_t;

struct KeyValuePair
{
	stdstr_t mKey;
	stdstr_t mValue;
};