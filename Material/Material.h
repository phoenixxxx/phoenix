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
		float3 Execute(uint32_t node);

		//Node creation
		uint32_t CreateNode(eNodeType type);

		//data
		void SetFloat(uint32_t node, float x);
		void SetFloat2(uint32_t node, float x, float y);
		void SetFloat3(uint32_t node, float x, float y, float z);
		void SetFloat4(uint32_t node, float x, float y, float z, float w);
		void SetInt(uint32_t node, int x);
		void SetInt2(uint32_t node, int x, int y);
		void SetInt3(uint32_t node, int x, int y, int z);
		void SetInt4(uint32_t node, int x, int y, int z, int w);

		//sampler 2D
		void SetSamplerTexcoords(uint32_t node, uint32_t texCoordsNode);
		void SetSamplerAddressing(uint32_t node);///, imageSampler2DAddressing_t addressing);
		void SetSamplerFiltering(uint32_t node);// , imageSampler2DFilter_t filtering);

		//eLambertianBRDF
		void SetDiffuseBRDFAlbedo(uint32_t node, uint32_t albedoNode);
		void SetDiffuseBRDFNormal(uint32_t node, uint32_t normalNode);
		void SetDiffuseBRDFRoughness(uint32_t node, uint32_t roughnessNode);

		//math
		void SetMathInputA(uint32_t node, uint32_t a);
		void SetMathInputB(uint32_t node, uint32_t b);

	private:
		std::vector<SharedMaterialNode> mNodes;
	};
}
