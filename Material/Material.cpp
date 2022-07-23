#include <stack>
#include "Material.h"
#include <SharedGPU_CPU/SharedHierarchy.h>//INVALID_INDEX def

#define MAX_STACK_DEPTH 16
#define STACK_RETURN {assert(stackIndex > 0); stackIndex--;}

#define STACK_POP_F(v)                        \
{                                             \
	assert(stackIndex > 0);                   \
	stackIndex--;                             \
	stack[stackIndex].mFloat[0] = v;          \
	stack[stackIndex].mInputAvailable = true; \
}                                             \
											  
#define STACK_POP_F2(v)                       \
{                                             \
	assert(stackIndex > 0);                   \
	stackIndex--;                             \
	stack[stackIndex].mFloat[0] = v.x;        \
	stack[stackIndex].mFloat[1] = v.y;        \
	stack[stackIndex].mInputAvailable = true; \
}                                             \
											  
#define STACK_POP_F3(v)                       \
{                                             \
	assert(stackIndex > 0);                   \
	stackIndex--;                             \
	stack[stackIndex].mFloat[0] = v.x;        \
	stack[stackIndex].mFloat[1] = v.y;        \
	stack[stackIndex].mFloat[2] = v.z;        \
	stack[stackIndex].mInputAvailable = true; \
}                                             \
											  
#define STACK_POP_F4(v)                       \
{                                             \
	assert(stackIndex > 0);                   \
	stackIndex--;                             \
	stack[stackIndex].mFloat[0] = v.x;        \
	stack[stackIndex].mFloat[1] = v.y;        \
	stack[stackIndex].mFloat[2] = v.z;        \
	stack[stackIndex].mFloat[3] = v.w;        \
	stack[stackIndex].mInputAvailable = true; \
}                                             \

#define READ_F(f)                        \
{                                        \
	f = stack[stackIndex].mFloat[0];     \
}                                        \

#define READ_F2(v)                       \
{                                        \
	v.x = stack[stackIndex].mFloat[0];   \
	v.y = stack[stackIndex].mFloat[1];   \
}                                        \
										 
#define READ_F3(v)                       \
{                                        \
	v.x = stack[stackIndex].mFloat[0];   \
	v.y = stack[stackIndex].mFloat[1];   \
	v.z = stack[stackIndex].mFloat[2];   \
}                                        \
										 
#define READ_F4(v)                       \
{                                        \
	v.x = stack[stackIndex].mFloat[0];   \
	v.y = stack[stackIndex].mFloat[1];   \
	v.z = stack[stackIndex.mFloat[2];   \
	v.z = stack[stackIndex].mFloat[3];   \
}                                        \

namespace Phoenix
{
	MaterialSystem::MaterialSystem(){}

	MaterialSystem* MaterialSystem::Instance()
	{
		static MaterialSystem instance;
		return &instance;
	}

	static void ExecuteMathFunction(
		const SharedMaterialNode& node,
		uint32_t currentInputIndex,
		sStackInfo stack[],
		uint32_t& stackIndex)
	{
		static float4 A, outValue;
		if (INPUT_AVAILABLE)
		{
			//READ_F4(A);
			//do the math!
			switch (node.mType)
			{
			case eNodeType::eMathOpSin:
			{
				outValue.x = sinf(A.x);
			}break;
			case eNodeType::eMathOpCos:
			{
				outValue.x = cosf(A.x);
			}break;
			case eNodeType::eMathOpTan:
			{
				outValue.x = tanf(A.x);
			}break;
			case eNodeType::eMathOpASin:
			{
				outValue.x = asinf(A.x);
			}break;
			case  eNodeType::eMathOpACos:
			{
				outValue.x = acosf(A.x);
			}break;
			case  eNodeType::eMathOpATan:
			{
				outValue.x = atanf(A.x);
			}break;
			}
			//STACK_POP_F4(outValue);
		}
		else
		{
			STACK_PUSH(node.mInputs[eMathOpsInputs::A]);
		}
	}

	static void ExecuteMathOperators(
		const SharedMaterialNode& node,
		uint32_t currentInputIndex,
		sStackInfo stack[],
		uint32_t& stackIndex)
	{
		static float4 A, B, outValue;
		switch (currentInputIndex)
		{
		case eMathOpsInputs::A:
		{
			if (INPUT_AVAILABLE)
			{
				//READ_F4(A);
				NEXT_INPUT(currentInputIndex);
			}
			else
			{
				STACK_PUSH(node.mInputs[eMathOpsInputs::A]);
			}
		}break;
		case eMathOpsInputs::B:
		{
			if (INPUT_AVAILABLE)
			{
				//READ_F4(B);
				//do the math!
				switch (node.mType)
				{
				case eNodeType::eMathOpAdd:
				{
					outValue = A + B;
				}break;
				case eNodeType::eMathOpSub:
				{
					outValue = A - B;
				}break;
				case eNodeType::eMathOpMul:
				{
					outValue = A * B;
				}break;
				case eNodeType::eMathOpDiv:
				{
					outValue.x = A.x / B.x;
					outValue.y = A.y / B.y;
					outValue.z = A.z / B.z;
					outValue.w = A.w / B.w;
				}break;
				}
				//STACK_POP_F4(outValue);
			}
			else
			{
				STACK_PUSH(node.mInputs[eMathOpsInputs::B]);
			}
		}break;
		}
	}

	static void ExecuteDiffseBRDF(
		const SharedMaterialNode& node,
		uint32_t currentInputIndex,
		sStackInfo stack[],
		uint32_t& stackIndex)
	{
		static float3 albedo, normal;
		static float roughness;

		switch (currentInputIndex)
		{
		case eBRDFDiffuseInputs::eAlbedo:
		{
			if (INPUT_AVAILABLE)
			{
				//READ_F3(albedo);
				NEXT_INPUT(currentInputIndex);
			}
			else
			{
				STACK_PUSH(node.mInputs[eBRDFDiffuseInputs::eAlbedo]);
			}
		}break;
		case eBRDFDiffuseInputs::eNormal:
		{
			if (INPUT_AVAILABLE)
			{
				//READ_F3(normal);
				NEXT_INPUT(currentInputIndex);
			}
			else
			{
				STACK_PUSH(node.mInputs[eBRDFDiffuseInputs::eNormal]);
			}
		}break;
		case eBRDFDiffuseInputs::eRoughness:
		{
			if (INPUT_AVAILABLE)
			{
				//READ_F(roughness);
				//do the BRDF comp
				//STACK_POP_F3(float4(0.123f, 0.456f, 0.789f));
			}
			else
			{
				STACK_PUSH(node.mInputs[eBRDFDiffuseInputs::eRoughness]);
			}
		}break;
		}
	}

	float3 MaterialSystem::Execute(uint32_t rootNodeIndex)
	{
		sStackInfo stack[MAX_STACK_DEPTH];//<-- max depth
		uint32_t stackIndex = 1;

		//push root
		stack[stackIndex].mNodeIndex = rootNodeIndex;
		stack[stackIndex].mInputIndex = 0;
		stack[stackIndex].mInputAvailable = 0;

		do
		{
			auto& current = STACK_TOP;
			SharedMaterialNode& node = mNodes[current.mNodeIndex];

			switch (node.mType)
			{
			case eNodeType::eMathOpSin:
			case eNodeType::eMathOpCos:
			case eNodeType::eMathOpTan:
			case eNodeType::eMathOpASin:
			case eNodeType::eMathOpACos:
			case eNodeType::eMathOpATan:
			{
				ExecuteMathFunction(node, current.mInputIndex, stack, stackIndex);
			}break;
			case eNodeType::eMathOpAdd:
			case eNodeType::eMathOpSub:
			case eNodeType::eMathOpMul:
			case eNodeType::eMathOpDiv:
			{
				ExecuteMathOperators(node, current.mInputIndex, stack, stackIndex);
			}break;
			case eNodeType::eBRDFDiffuse:
			{
				ExecuteDiffseBRDF(node, current.mInputIndex, stack, stackIndex);
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
			case eNodeType::eFloat:
			{
				//STACK_POP_F(node.mFloat[0]);
			}
			break;
			}
		} while (!STACK_EMPTY);

		return float3();// stack[0].mFloat[0], stack[0].mFloat[1], stack[0].mFloat[2]);
	}

	//Node creation
	uint32_t MaterialSystem::CreateNode(eNodeType type)
	{
		uint32_t index = mNodes.size();
		SharedMaterialNode node;
		for (auto& input : node.mInputs)
			input = INVALID_INDEX;
		node.mType = type;
		mNodes.push_back(node);
		return index;
	}

	//data setters
	void MaterialSystem::SetFloat(uint32_t node, float x)
	{
		//mNodes[node].mFloat[0] = x;
	}
	void MaterialSystem::SetFloat2(uint32_t node, float x, float y)
	{
		//mNodes[node].mFloat[0] = x;
		//mNodes[node].mFloat[1] = y;
	}
	void MaterialSystem::SetFloat3(uint32_t node, float x, float y, float z)
	{
		//mNodes[node].mFloat[0] = x;
		//mNodes[node].mFloat[1] = y;
		//mNodes[node].mFloat[2] = z;
	}
	void MaterialSystem::SetFloat4(uint32_t node, float x, float y, float z, float w)
	{
		//mNodes[node].mFloat[0] = x;
		//mNodes[node].mFloat[1] = y;
		//mNodes[node].mFloat[2] = z;
		//mNodes[node].mFloat[3] = w;
	}

	void MaterialSystem::SetInt(uint32_t node, int x)
	{
		//mNodes[node].mInt[0] = x;
	}
	void MaterialSystem::SetInt2(uint32_t node, int x, int y)
	{
		//mNodes[node].mInt[0] = x;
		//mNodes[node].mInt[1] = y;
	}
	void MaterialSystem::SetInt3(uint32_t node, int x, int y, int z)
	{
		//mNodes[node].mInt[0] = x;
		//mNodes[node].mInt[1] = y;
		//mNodes[node].mInt[2] = z;
	}
	void MaterialSystem::SetInt4(uint32_t node, int x, int y, int z, int w)
	{
		//mNodes[node].mInt[0] = x;
		//mNodes[node].mInt[1] = y;
		//mNodes[node].mInt[2] = z;
		//mNodes[node].mInt[3] = w;
	}

	//sampler 2D
	void MaterialSystem::SetSamplerTexcoords(uint32_t node, uint32_t texCoordsNode)
	{
		//mNodes[node].mInputs[NODE_TYPE_IMAGE_SAMPLER_2D_INPUT_UV] = texCoordsNode;
	}
	void MaterialSystem::SetSamplerAddressing(uint32_t node)//, imageSampler2DAddressing_t addressing)
	{
		//mNodes[node].mImageSampler2D.mAddressing = addressing;
	}
	void MaterialSystem::SetSamplerFiltering(uint32_t node)//, imageSampler2DAddressing_t filtering)
	{
		//mNodes[node].mImageSampler2D.mFilter = filtering;
	}

	//eLambertianBRDF
	void MaterialSystem::SetDiffuseBRDFAlbedo(uint32_t node, uint32_t albedoNode)
	{
		mNodes[node].mInputs[eBRDFDiffuseInputs::eAlbedo] = albedoNode;
	}
	void MaterialSystem::SetDiffuseBRDFNormal(uint32_t node, uint32_t normalNode)
	{
		mNodes[node].mInputs[eBRDFDiffuseInputs::eNormal] = normalNode;
	}
	void MaterialSystem::SetDiffuseBRDFRoughness(uint32_t node, uint32_t roughnessNode)
	{
		mNodes[node].mInputs[eBRDFDiffuseInputs::eRoughness] = roughnessNode;
	}

	void MaterialSystem::SetMathInputA(uint32_t node, uint32_t a)
	{
		mNodes[node].mInputs[eMathOpsInputs::A] = a;
	}
	void MaterialSystem::SetMathInputB(uint32_t node, uint32_t b)
	{
		mNodes[node].mInputs[eMathOpsInputs::B] = b;
	}
}
