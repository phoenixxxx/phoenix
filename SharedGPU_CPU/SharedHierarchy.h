#ifndef GPU_SHARED_HIERARCHY_H
#define GPU_SHARED_HIERARCHY_H

#define INVALID_INDEX ((unsigned int)(-1))

#if defined CPU_ENVIRONMENT
#include <Utils/VectorMath.h>
#endif 

typedef struct GPUBVHNode
{
	//relationship
	uint64_t  mParent;
	uint64_t  mLeftChild;
	uint64_t  mRightChild;

	//leaf node
	uint64_t  mLeafData;

	//AABB
	float4 mBoxMin;
	float4 mBoxMax;

}GPUBVHNode;

typedef struct GPUBVHLightingNode
{
	//relationship
	uint64_t  mParent;
	uint64_t  mLeftChild;
	uint64_t  mRightChild;

	//leaf node
	uint64_t  mLeafData;

	//AABB
	float4 mBoxMin;
	float4 mBoxMax;

	//lighting data
	float4 mConeInfo;//XYZ:direction and W:angle of emissive cone
	float4 mFlux;
}GPUBVHLightingNode;

#endif//GPU_SHARED_HIERARCHY_H
