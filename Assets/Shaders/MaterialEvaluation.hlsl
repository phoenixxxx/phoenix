
#include <SharedMaterial.h>

StructuredBuffer<SharedMaterialNode> nodes;

struct GlobalInput 
{
	unsigned int mRayCount;
};
StructuredBuffer<GlobalInput> globals;

[numthreads(PRIMARY_RAY_LOCAL_SIZE, 1, 1)]
void Evaluate(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	int id = dispatchThreadID.x;
	if (id >= globals[0].mRayCount)
		return;
}