
#include <SharedMaterial.h>
#include <SharedHierarchy.h>
#include <SharedGeometry.h>

StructuredBuffer<SharedMaterialNode> gNodes;
StructuredBuffer<GPUPath> gPaths;
struct GlobalInput 
{
	unsigned int mRayCount;
};
StructuredBuffer<GlobalInput> globals;

#define STACK_POP_F(v)                        \
{                                             \
	stackIndex--;                             \
	stack[stackIndex].mData[0] = asuint(v);   \
	stack[stackIndex].mInputAvailable = true; \
}                                             \

[numthreads(PRIMARY_RAY_LOCAL_SIZE, 1, 1)]
void Evaluate(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	int id = dispatchThreadID.x;
	if (id >= globals[0].mRayCount)
		return;

	uint32_t rootNodeIndex = gPaths[id].mMaterialNodeRoot;

	sStackInfo stack[MAX_STACK_DEPTH];
	uint32_t stackIndex = 1;

	//push root
	stack[stackIndex].mNodeIndex = rootNodeIndex;
	stack[stackIndex].mInputIndex = 0;
	stack[stackIndex].mInputAvailable = 0;

	do
	{
		sStackInfo current = STACK_TOP;
		SharedMaterialNode node = gNodes[current.mNodeIndex];

		switch (node.mType)
		{
		case eNodeType::eMathOpSin:
		case eNodeType::eMathOpCos:
		case eNodeType::eMathOpTan:
		case eNodeType::eMathOpASin:
		case eNodeType::eMathOpACos:
		case eNodeType::eMathOpATan:
		{
			//ExecuteMathFunction(node, current.mInputIndex, stack, stackIndex);
		}break;
		case eNodeType::eMathOpAdd:
		case eNodeType::eMathOpSub:
		case eNodeType::eMathOpMul:
		case eNodeType::eMathOpDiv:
		{
			//ExecuteMathOperators(node, current.mInputIndex, stack, stackIndex);
		}break;
		case eNodeType::eBRDFDiffuse:
		{
			//ExecuteDiffseBRDF(node, current.mInputIndex, stack, stackIndex);
		}
		break;
		case eNodeType::eImageSampler2D:
		{
			float2 texCoords;
			if (INPUT_AVAILABLE)
			{
				//use the texcoords
				//READ_F2(texCoords);
				//STACK_POP_F3(float4(0.1f, 0.2f, 0.3f));
			}
			else
			{
				STACK_PUSH(node.mInputs[eImageSampler2DInputs::UV]);
			}
		}
		break;
		case eNodeType::eObjNml:
		{
			//write out normal
			///STACK_POP_F3(float4(0.5f, 0.5f, 0.5f));
		}
		break;
		case eNodeType::eObjUV0:
		{
			//write out texcoords
			//STACK_POP_F2(float4(0, 1));
		}
		break;
		case eNodeType::eObjTan:
		{
		}
		break;
		case eNodeType::eObjBitan:
		{
		}
		break;
		case eNodeType::eObjPos:
		{
		}
		break;

		case eNodeType::eFloat:
		{
			STACK_POP_F(node.mData[0]);
		}
		break;
		case eNodeType::eUnsigned:
		{
		}
		break;
		case eNodeType::eInt:
		{
		}
		break;
		}
	} while (!STACK_EMPTY);

}
