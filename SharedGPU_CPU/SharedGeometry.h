#ifndef SharedGPU_CPU_Geometry_H
#define SharedGPU_CPU_Geometry_H

#if defined CPU_ENVIRONMENT
#include <Utils/VectorMath.h>
#endif 

struct Vertex
{
	float4 mPosition;
	float4 mNormal;
	float4 mUV;//UV0, UV1
};

struct Instance
{
	float4x4 mToWorldGlobalNormal;//bring a normal into global space
	float4x4 mToWorldGlobal;//bring the local ray into global space
	float4x4 mToInstanceLocal;//bring the ray into local space of the instance
	uint32_t mMeshEntry;
};

struct MeshEntry
{
	uint64_t mRootBVH;
	uint32_t mVertexOffset;
	uint32_t mIndexOffset;
};

typedef struct
{
	float4 mPosition;
	//This is used to ensure the ray does not 
	//hit the same primitive when bouncing off of it
	float4 mOffset;

	float4 mWo;//outpgoing direction
	float4 mWi;//incoming direction

	float4 mLi;//incoming radiance

	float4 mLo;//outputted radiance
	float4 mIndirectThroughput;//compounded radiance factor [f()*cos(theta)]/pdf

	uint32_t mMaterialNodeRoot;
	uint32_t mPadding[3];
}GPUPath;

#endif //SharedGPU_CPU_Geometry_H