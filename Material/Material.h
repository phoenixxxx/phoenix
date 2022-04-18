#pragma once

#include <vector>
#include "MaterialNode.h"
#include <SharedGPU_CPU/SharedMaterial.h>

namespace Phoenix
{
	class MaterialSystem
	{
	private:
		MaterialSystem();
	public:
		static MaterialSystem* Instance();
	public:
		template <typename T>
		T* AllocateNode()
		{
			T* node = new T();
			mNodes.push_back(node);
			return node;
		}

		void Compile(const MaterialNode::Plug& output);
	private:
		struct StackState
		{
			uint32_t mNode;
			uint32_t mOutputPlug;

			uint32_t mShaderGraphState;
			uint32_t mCurrentInputPlug;

			float mValue[4];

			union
			{
				struct
				{
					float mTexCoords[4];
				}mTex2D;
				struct
				{
					float mAlbedo[4];
					float mNormal[4];
					float mRoughness[4];
				}mDiffuseBSDF;
			};
		};
		bool ReadPlugData(float output[4], float defaultValue[4], float4 valueOutput, StackState& current, StackState& next, const SharedMaterialNode::Plug& plug);
		void ConvertNodes();
		uint32_t GetNodeIndex(MaterialNode* node);
		void ConvertPlug(SharedMaterialNode::Plug& outputPlug, const MaterialNode::Plug& inuputPlug);

	private:
		std::vector<MaterialNode*> mNodes;
		std::vector<SharedMaterialNode> mSharedNodes;
	};
}
