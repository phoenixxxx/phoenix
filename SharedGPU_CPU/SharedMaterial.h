#ifndef SharedGPU_CPU_Material_H
#define SharedGPU_CPU_Material_H

#define MAX_NODE_INPUT_COUNT 8

//Sampler 2D inputs
#define NODE_TYPE_IMAGE_SAMPLER_2D_INPUT_UV 0

//Sampler 2D filters
typedef unsigned int uint32_t;
#define NODE_TYPE_IMAGE_SAMPLER_2D_FILTER_NEAREST 0
#define NODE_TYPE_IMAGE_SAMPLER_2D_FILTER_LINEAR  1
//Sampler 2D addressing
#define NODE_TYPE_IMAGE_SAMPLER_2D_ADDR_WRAP  0
#define NODE_TYPE_IMAGE_SAMPLER_2D_ADDR_CLAMP 1

//Diffuse BRDF inputs
#define NODE_TYPE_BRDF_DIFFUSE_INPUT_ALBEDO    0
#define NODE_TYPE_BRDF_DIFFUSE_INPUT_NORMAL    1
#define NODE_TYPE_BRDF_DIFFUSE_INPUT_ROUGHNESS 2

//Math ops inputs
#define NODE_TYPE_MATHOP_INPUT_A  0
#define NODE_TYPE_MATHOP_INPUT_B  1

//Node types
typedef uint32_t nodeType_t;
#define NODE_TYPE_FLOAT       0
#define NODE_TYPE_MATHOP_ADD  1
#define NODE_TYPE_MATHOP_SUB  2
#define NODE_TYPE_MATHOP_MUL  3
#define NODE_TYPE_MATHOP_DIV  4

#define NODE_TYPE_MATHOP_SIN  5
#define NODE_TYPE_MATHOP_COS  6
#define NODE_TYPE_MATHOP_TAN  7

#define NODE_TYPE_MATHOP_ASIN 8
#define NODE_TYPE_MATHOP_ACOS 9
#define NODE_TYPE_MATHOP_ATAN 10

#define NODE_TYPE_IMAGE_SAMPLER_2D 11

#define NODE_TYPE_OBJ_POS    12
#define NODE_TYPE_OBJ_NML    13
#define NODE_TYPE_OBJ_TAN    14
#define NODE_TYPE_OBJ_BITAN  15
#define NODE_TYPE_OBJ_UV0    16

#define NODE_TYPE_BRDF_DIFFUSE 17

struct sStackInfo
{
	uint32_t mNodeIndex;
	uint32_t mInputIndex;
	uint32_t mInputAvailable;

	uint32_t mData[4];
};

#define SHARED_MATERIAL_DATA_FIELD 1
struct SharedMaterialNode
{
	uint32_t   mInputs[MAX_NODE_INPUT_COUNT];
	nodeType_t mType;

	//data
	uint32_t mData[4 * SHARED_MATERIAL_DATA_FIELD];
};

#endif//SharedGPU_CPU_Material_H
