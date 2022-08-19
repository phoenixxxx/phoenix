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

enum class GeometryType {
	eTriangularMesh,
	eSphere,
	//eCone,
	//eBox,
	//ePlane
};

struct Instance
{
	float4x4 mToWorldGlobalNormal;//bring a normal into global space
	float4x4 mToWorldGlobal;//bring the local ray into global space
	float4x4 mToInstanceLocal;//bring the ray into local space of the instance
	GeometryType mGeometryType;
	uint32_t mGeometryIndex;
};

struct MeshEntry
{
	uint64_t mRootBVH;
	uint32_t mVertexOffset;
	uint32_t mIndexOffset;
};

struct ProceduralGeometry
{
	float mData[4 * 2];//8flt data
};

#endif //SharedGPU_CPU_Geometry_H
