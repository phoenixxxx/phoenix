#include <stack>
#include "Material.h"
#include <SharedGPU_CPU/SharedHierarchy.h>//INVALID_INDEX def

namespace Phoenix
{
	MaterialSystem::MaterialSystem(){}

	MaterialSystem* MaterialSystem::Instance()
	{
		static MaterialSystem instance;
		return &instance;
	}

	uint32_t MaterialSystem::GetNodeIndex(MaterialNode* node)
	{
		uint32_t index = 0;
		for (auto iNode : mNodes)
		{
			if (iNode == node)
				break;
			index++;
		}
		return index;
	}

	void MaterialSystem::ConvertPlug(SharedMaterialNode::Plug& outputPlug, const MaterialNode::Plug& inuputPlug)
	{
		if (inuputPlug != nullptr)
		{
			outputPlug.mNode = GetNodeIndex(inuputPlug.mSlot->mNode);
			outputPlug.mSlot = inuputPlug.mSlot->mIndex;
		}
		else
		{
			outputPlug.mNode = INVALID_INDEX;
			outputPlug.mSlot = INVALID_INDEX;
		}
	}

	void MaterialSystem::ConvertNodes()
	{
		//convert the node to SharedMaterialNode
		mSharedNodes.resize(mNodes.size());
		for (uint32_t iNode = 0; iNode < mNodes.size(); ++iNode)
		{
			MaterialNode::Type nodeType = mNodes[iNode]->GetType();
			mSharedNodes[iNode].mType = (uint32_t)(nodeType);
			if (nodeType == MaterialNode::Type::eScalar)
			{
				MaterialNodeScalar* scalarNode = static_cast<MaterialNodeScalar*>(mNodes[iNode]);
				
				mSharedNodes[iNode].mValue = float4(scalarNode->mValue, 0, 0, 1);
				ConvertPlug(mSharedNodes[iNode].mPlugs[0], scalarNode->GetValuePlug());
			}
			else if (nodeType == MaterialNode::Type::eVector)
			{
				MaterialNodeVector* rgbaNode = static_cast<MaterialNodeVector*>(mNodes[iNode]);

				mSharedNodes[iNode].mValue = rgbaNode->mValue;
				ConvertPlug(mSharedNodes[iNode].mPlugs[0], rgbaNode->GetValuePlug());
			}
			else if (nodeType == MaterialNode::Type::eTexture2D)
			{
				MaterialNodeTexture2D* tex2DNode = static_cast<MaterialNodeTexture2D*>(mNodes[iNode]);

				//TexCoords input
				ConvertPlug(mSharedNodes[iNode].mPlugs[0], tex2DNode->GetTexCoordsPlug());

				//RGBA output
				ConvertPlug(mSharedNodes[iNode].mPlugs[1], tex2DNode->GetColorPlug());

				//Alpha output
				ConvertPlug(mSharedNodes[iNode].mPlugs[2], tex2DNode->GetAlphaPlug());
			}
			else if (nodeType == MaterialNode::Type::eDiffuseBSDF)
			{
				MaterialNodeDiffuseBSDF* diffBSDFNode = static_cast<MaterialNodeDiffuseBSDF*>(mNodes[iNode]);

				//Albedo input
				ConvertPlug(mSharedNodes[iNode].mPlugs[0], diffBSDFNode->GetAlbedoPlug());

				//Normal input
				ConvertPlug(mSharedNodes[iNode].mPlugs[1], diffBSDFNode->GetNormalPlug());

				//Roughness input
				ConvertPlug(mSharedNodes[iNode].mPlugs[2], diffBSDFNode->GetRoughnessPlug());

				//BSDF output
				ConvertPlug(mSharedNodes[iNode].mPlugs[3], diffBSDFNode->GetBSDFPlug());
			}
		}
	}

	bool MaterialSystem::ReadPlugData(float output[4], float defaultValue[4], float4 valueOutput, StackState& current, StackState& next, const SharedMaterialNode::Plug& plug)
	{
		if (current.mShaderGraphState == SGS_WAITING_FOR_PLUG)
		{
			//we are waiting for the plug so value stores the returned value
			output[0] = valueOutput.x;
			output[1] = valueOutput.y;
			output[2] = valueOutput.z;
			output[3] = valueOutput.w;

			current.mShaderGraphState = SGS_RUNNING;
			current.mCurrentInputPlug++;

			return true;
		}
		if (plug.mNode == INVALID_INDEX)
		{
			output[0] = defaultValue[0];
			output[1] = defaultValue[1];
			output[2] = defaultValue[2];
			output[3] = defaultValue[3];
			
			current.mCurrentInputPlug++;
			return true;
		}

		next.mNode = plug.mNode;
		next.mOutputPlug = plug.mSlot;
		next.mCurrentInputPlug = 0;
		next.mShaderGraphState = SGS_RUNNING;

		current.mShaderGraphState = SGS_WAITING_FOR_PLUG;

		return false;
	}

	void MaterialSystem::Compile(const MaterialNode::Plug& outPlug)
	{
		if(mSharedNodes.size() == 0)
			ConvertNodes();

		float4 valueOutput;
		std::stack<StackState> stackNode;
		StackState seed;
		seed.mNode = GetNodeIndex(outPlug.mNode);
		seed.mOutputPlug = outPlug.mIndex;
		seed.mShaderGraphState = SGS_RUNNING;
		seed.mCurrentInputPlug = 0;
		stackNode.push(seed);

		while (!stackNode.empty())
		{
			StackState& current = stackNode.top();
			SharedMaterialNode& node = mSharedNodes[current.mNode];
			switch (node.mType)
			{
			case MATERIAL_NODE_TYPE_VECTOR:
			case MATERIAL_NODE_TYPE_SCALAR:
			{
				//put node value onto the stack
				valueOutput.x = node.mValue.x;
			}break;
			case MATERIAL_NODE_TYPE_TEX2D:
			{
				if (current.mCurrentInputPlug == 0)//Albedo
				{
					StackState next;
					float interactionUVs[4] = { 0.5f, 0.5f, 0, 0 };//store the normal value from the interaction
					if (ReadPlugData(current.mTex2D.mTexCoords, interactionUVs, valueOutput, current, next, node.mPlugs[0]) == false)
					{
						//failed to read, recurse
						stackNode.push(next);
						continue;
					}
				}
				//we have made it here, we have all the data we need
				valueOutput = float4(0.4f, 0.3f, 0.2f, 0.1f);
			}break;
			case MATERIAL_NODE_TYPE_DIFFUSE_BSDF:
			{
				if (current.mCurrentInputPlug == 0)//Albedo
				{
					StackState next;
					if (ReadPlugData(current.mDiffuseBSDF.mAlbedo, nullptr, valueOutput, current, next, node.mPlugs[0]) == false)
					{
						//failed to read, recurse
						stackNode.push(next);
						continue;
					}
				}
				if (current.mCurrentInputPlug == 1)//Normal
				{
					StackState next;
					float interactionNormal[4] = { 1, 0, 0, 1 };//store the normal value from the interaction
					if (ReadPlugData(current.mDiffuseBSDF.mNormal, interactionNormal, valueOutput, current, next, node.mPlugs[1]) == false)
					{
						//failed to read, recurse
						stackNode.push(next);
						continue;
					}
				}
				if (current.mCurrentInputPlug == 2)//Roughness
				{
					StackState next;
					if (ReadPlugData(current.mDiffuseBSDF.mRoughness, nullptr, valueOutput, current, next, node.mPlugs[2]) == false)
					{
						//failed to read, recurse
						stackNode.push(next);
						continue;
					}
				}
				//we have made it here, we have all the data we need
				valueOutput = float4(0.1f, 0.2f, 0.3f, 0.4f);
			}break;
			}

			stackNode.pop();
		}

	}
}
