#ifndef __SHARED_SCENE_HLSL__
#define __SHARED_SCENE_HLSL__

#if defined CPU_ENVIRONMENT
#include <Utils/VectorMath.h>
#endif 

#define PCG32_DEFAULT_STATE   0x853c49e6748fea9bULL
#define PCG32_DEFAULT_STREAM  0xda3e39cb94b95bdbULL
#define PCG32_MULT            0x5851f42d4c957f2dULL//HEX 6364136223846793005ULL

struct PCG32RNG
{
	uint64_t mState; // RNG state.  All values are possible.
	uint64_t mInc;   // Controls which RNG sequence (stream) is selected. Must *always* be odd.
};

struct GPUCamera
{
	float mFocusDistance;//distance to the focal plane from sensor
	float mShutterSpeed;
	float mLenseRadius;
	float mFocalLength;

	float2 mSensorDimension;//Sensor height is calculated by accounting image aspect ratio (vertical film size = horizontal film size / aspect ratio).
	uint2  mSensorResolution;

	float4x4 mTransform;
};

struct GPUHit
{
	float4 mWo;
	float4 mN;
	float4 mPos;
	float4 mUV;

	float4 mDpDx, mDpDy;
	float4 mDpDu, mDpDv;
	float4 mDnDu, mDnDv;

	struct
	{
		float4 mN;
		float4 mDpDu, mDpDv;
		float4 mDnDu, mDnDv;
	}mShading;

	float     mTime;
	uint32_t  mPrimitive;
	uint32_t  mInstance;
};

struct GPUPath
{
	float4 mOrigin;
	float4 mOffset;//shift from the current surface
	float4 mDirection;

	//ray differentials
	float4 mRxDirection, mRxOrigin;
	float4 mRyDirection, mRyOrigin;

	/*
	x: 0=Ray dead, 1=Ray alive
	y: 0=Don't Sample background, 1=Sample Backgroun
	*/
	int4   mStates;

	PCG32RNG mRNG;//int4 size
};


#if defined CPU_ENVIRONMENT
#define GPUPATHINOUT GPUPath&
#else
#define GPUPATHINOUT inout GPUPath
#endif
static inline void SetRayIsAlive(GPUPATHINOUT path)
{
	path.mStates.x = 1;
}

static inline bool RayIsAlive(GPUPATHINOUT path)
{
	return (path.mStates.x == 1);
}

#endif //__SHARED_SCENE_HLSL__