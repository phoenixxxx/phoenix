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

#endif //SharedGPU_CPU_Geometry_H