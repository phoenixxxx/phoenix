#ifndef __SHARED_IMAGE_HLSL__
#define __SHARED_IMAGE_HLSL__

#define MAX_MIPS 13
#define MAX_RESOLUTION_DIM (1 << (MAX_MIPS - 1))

enum class Format {
	eUnknown,
	eFloat3,
	eFloat4,
	eUbyte3,
	eUbyte4,
};

enum class Filter {
	eNearest,
	eLinear
};

enum class Boundaries {
	eWrap,
	eClamp,
};

struct ImageBlock
{
	Format mFormat;
	uint32_t mImageOffset;//offset in the image byte buffer
	uint32_t mMipsCount;
	struct
	{
		uint32_t mTexelsOffset;//offset in the buffer to the mip
		Size mResolution;
	}mMips[MAX_MIPS];//
};

#endif //__SHARED_IMAGE_HLSL__
