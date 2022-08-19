#ifndef RNG_HLSL
#define RNG_HLSL

#include <SharedScene.h>

static uint32_t rngGetUniformUint32(inout PCG32RNG rng)
{
	uint64_t oldstate = rng.mState;
	rng.mState = oldstate * PCG32_MULT + rng.mInc;
	uint32_t xorshifted = (uint32_t)(((oldstate >> 18u) ^ oldstate) >> 27u);
	uint32_t rot = (uint32_t)(oldstate >> 59u);
	return (xorshifted >> rot) | (xorshifted << ((~rot + 1U) & 31));
}

static void rngSetSequence(inout PCG32RNG rng, uint64_t initseq, uint64_t initstate)
{
	rng.mState = 0U;
	rng.mInc = (initseq << 1u) | 1u;
	rngGetUniformUint32(rng);
	rng.mState += initstate;
	rngGetUniformUint32(rng);
}

static float rngGetUniformFloat(inout PCG32RNG rng)
{
	return ldexp((float)rngGetUniformUint32(rng), -32);
}

#endif //RNG_HLSL