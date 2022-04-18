#pragma once
#include <cstdint>
#include <string>

typedef std::string stdstr_t;
typedef const char* cstr_t;
typedef uint8_t byte_t;

struct Size
{
	uint32_t mWidth;
	uint32_t mHeight;
};

typedef uint32_t bitfield_t;
typedef uint64_t BVHNodeIndex;