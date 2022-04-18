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

#endif //SharedGPU_CPU_Geometry_H