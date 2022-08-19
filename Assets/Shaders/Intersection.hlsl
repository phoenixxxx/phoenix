#include <SharedTypes.h>
#include <SharedScene.h>
#include <SharedHierarchy.h>
#include <SharedGeometry.h>
#include <IntersectionShared.h>

#include "geometry.hlsl"

RWStructuredBuffer<GPUHit>    gHits;
StructuredBuffer<GPUPath>     gPaths;
StructuredBuffer<GlobalInput> gInputs;

StructuredBuffer<Instance>   gInstances;
StructuredBuffer<GPUBVHNode> gTopLevelAS;

StructuredBuffer<ProceduralGeometry> gProceduralGeo;
StructuredBuffer<MeshEntry>          gMeshes;
StructuredBuffer<Vertex>             gVertexBuffer;
StructuredBuffer<int3>               gIndexBuffer;

[numthreads(INTERSECTION_LOCAL_SIZE, 1, 1)]
void Execute(uint3 dispatchThreadID : SV_DispatchThreadID, uint3 groupThreadID : SV_GroupThreadID)
{
	int id = dispatchThreadID.x;

	GPUPath path = gPaths[id];
	if (!RayIsAlive(path))
		return;

    Ray ray;
	ray.mOrigin = (path.mOrigin + path.mOffset).xyz;
	ray.mDirection = path.mDirection.xyz;

    BVHNodeIndex stackBuffer[LOCAL_STACK_SIZE];

    //push root onto the stack
	uint32_t stackIndex = 1;
    stackBuffer[stackIndex] = gInputs[0].mRootRef;

	float tMin = FLT_MAX;
	uint32_t hitInstance, hitPrimitive;
	while (stackIndex > 0)
	{
		BVHNodeIndex index = stackBuffer[stackIndex];
		GPUBVHNode node = gTopLevelAS[(uint32_t)index];//again here we hope the compiler will optimize and NOT copy the whole node

		AABB box;
		box.mMin = node.mBoxMin.xyz;
		box.mMax = node.mBoxMax.xyz;
		if (IntersectRayAABB(ray, box))
		{
			if (GPUBVHNodeIsLeaf(node))
			{
				//use leaf
				Instance instance = gInstances[(uint32_t)node.mLeafData];
				Ray localRay = TransformRay(ray, instance.mToInstanceLocal);

				float primT;
				switch(instance.mGeometryType)
				{
					case GeometryType::eTriangularMesh:
					{
						//intersect the bottom level bvh
					}break;
					case GeometryType::eSphere:
					{
						primT = RayVsSphere(localRay);
					}break;
				}

				//primitive hit time is lower, store it
				if (primT < tMin)
				{
					tMin = primT;
					hitInstance = (uint32_t)node.mLeafData;
				}

				stackIndex--;//no matter what, we need to pop this node at this point
			}
			else
			{
				/*
				Note: This is squashing the node
				Assume
				A---B
					|
					C
				Stack: [A], push A's children [B|C] (A is getting smashed by B)
				This is done so we don't ever revisit the same node.
				*/
				stackBuffer[stackIndex] = node.mLeftChild;
				stackIndex++;
				stackBuffer[stackIndex] = node.mRightChild;
			}
		}
		else
		{
			stackIndex--;
		}
	}

	//store hit time (no hit is FLT_MAX)
	gHits[id].mTime = tMin;

	//Same kernel used for shadow ray, for next step evaluation
#if STORE_HIT_DATA
	if (tMin < FLT_MAX)//There was a HIT
	{
		gHits[id].mPrimitive = hitPrimitive;
		gHits[id].mInstance = hitInstance;
	}
#endif //STORE_HIT_DATA
}