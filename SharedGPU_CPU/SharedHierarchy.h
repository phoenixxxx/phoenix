#ifndef GPU_SHARED_HIERARCHY_H
#define GPU_SHARED_HIERARCHY_H

#define INVALID_INDEX ((unsigned int)(-1))

#if defined CPU_ENVIRONMENT
#include <Utils/VectorMath.h>
#endif 

typedef struct GPUBVHNode
{
	//relationship
	BVHNodeIndex  mLeftChild;
	BVHNodeIndex  mRightChild;

	//leaf node
	uint64_t  mLeafData;

	//AABB
	float4 mBoxMin;
	float4 mBoxMax;

}GPUBVHNode;

typedef struct GPUBVHLightingNode
{
	//relationship
	BVHNodeIndex  mLeftChild;
	BVHNodeIndex  mRightChild;

	//leaf node
	uint64_t  mLeafData;

	//AABB
	float4 mBoxMin;
	float4 mBoxMax;

	//lighting data
	float4 mConeInfo;//XYZ:direction and W:angle of emissive cone
	float4 mFlux;
}GPUBVHLightingNode;


#if defined CPU_ENVIRONMENT
#define GPUBVHNODEINOUT GPUBVHNode&
#else
#define GPUBVHNODEINOUT inout GPUBVHNode
#endif

static inline bool GPUBVHNodeIsLeaf(GPUBVHNODEINOUT node)
{
	return (node.mLeafData != INVALID_INDEX);
}

#endif//GPU_SHARED_HIERARCHY_H
