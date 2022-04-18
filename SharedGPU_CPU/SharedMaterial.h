#ifndef SharedGPU_CPU_Material_H
#define SharedGPU_CPU_Material_H

#define MATERIAL_NODE_TYPE_TEX2D 0
#define MATERIAL_NODE_TYPE_VECTOR  1
#define MATERIAL_NODE_TYPE_SCALAR 2
#define MATERIAL_NODE_TYPE_DIFFUSE_BSDF 3

#define MAX_PLUG_COUNT 8

//Shader Graph State (SGS)
#define SGS_RUNNING 0
#define SGS_WAITING_FOR_PLUG 1

#if defined CPU_ENVIRONMENT
#include <Utils/VectorMath.h>
#endif 

struct SharedMaterialNode
{
#if defined CPU_ENVIRONMENT
	SharedMaterialNode()
	{
		/*
		 *  message : 'SharedMaterialNode::SharedMaterialNode(void)': function was implicitly deleted because
		 *  'SharedMaterialNode' has a variant data member 'SharedMaterialNode::mValue' with a non-trivial
		 *  default constructor
		*/
	}
#endif 

	unsigned int mType;
	struct Plug
	{
		unsigned int mNode;//the node this is connected to
		unsigned int mSlot;//the index of the slot within the connected node
	}mPlugs[MAX_PLUG_COUNT];

	//node specific
	union
	{
		float4 mValue;
		struct
		{
		}mTex2D;
		struct
		{

		}mDiffuseBSDF;
	};
};

#endif//SharedGPU_CPU_Material_H
