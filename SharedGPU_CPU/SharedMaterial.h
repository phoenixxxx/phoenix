#ifndef SharedGPU_CPU_Material_H
#define SharedGPU_CPU_Material_H

#define MAX_NODE_INPUT_COUNT 8

//Sampler 2D inputs
enum eImageSampler2DInputs
{
	UV = 0
};

//Sampler 2D filters
enum eImageSampler2DFilter
{
	eNearest = 0,
	eLinear
};

//Sampler 2D addressing
enum eImageSampler2DAddr
{
	eWrap = 0,
	eClamp
};

//Diffuse BRDF inputs
enum eBRDFDiffuseInputs
{
	eAlbedo = 0,
	eNormal,
	eRoughness
};

//Math ops inputs
enum eMathOpsInputs
{
	A = 0,
	B
};

//Node types
enum eNodeType
{
	eFloat, eUnsigned, eInt,
	eMathOpAdd, eMathOpSub, eMathOpMul, eMathOpDiv,
	eMathOpSin, eMathOpCos, eMathOpTan,
	eMathOpASin, eMathOpACos, eMathOpATan,
	eImageSampler2D,
	eObjPos, eObjNml, eObjTan, eObjBitan, eObjUV0,
	eBRDFDiffuse
};

struct sStackInfo
{
	uint32_t mNodeIndex;
	uint32_t mInputIndex;
	uint32_t mInputAvailable;

	uint32_t mData[4];//float4, int4
};

#define SHARED_MATERIAL_DATA_FIELD 1
struct SharedMaterialNode
{
	uint32_t  mInputs[MAX_NODE_INPUT_COUNT];
	eNodeType mType;

	//data
	uint32_t  mData[4 * SHARED_MATERIAL_DATA_FIELD];
};

#define STACK_EMPTY (stackIndex==0)
#define STACK_TOP (stack[stackIndex])
#define NEXT_INPUT(index) { STACK_TOP.mInputIndex = index + 1; STACK_TOP.mInputAvailable = false; }
#define INPUT_AVAILABLE (STACK_TOP.mInputAvailable)

#define STACK_PUSH(node)                   \
{                                          \
	stackIndex++;                          \
	stack[stackIndex].mNodeIndex = node;   \
	stack[stackIndex].mInputIndex = 0;     \
	stack[stackIndex].mInputAvailable = 0; \
}                                          \

#endif//SharedGPU_CPU_Material_H
