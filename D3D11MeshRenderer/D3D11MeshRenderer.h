#pragma once

#include <vector>
#include <Utils/Math.h>
#include <Utils/VectorMath.h>
#include <Utils/Types.h>
#include <D3D11/D3D11Manager.h>
#include <Camera/Camera.h>
#include "Utils/Singleton.h"
#include <SharedGPU_CPU/SharedGeometry.h>
#include <map>

namespace Phoenix
{
	class D3D11MeshRenderer
	{
	public:
		bool Initialize();
		void Shutdown();
		void Render(const Camera& camera);

		void DrawMesh(Vertex* vertexBuffer, size_t vtxCount, int3* indexBuffer, size_t faceCount, const float4x4& world, const float4x4& worldInversTrans, const float4& color);
		void DrawTriangle(const float3& p0, const float3& p1, const float3& p2, const float3& color);

		void ReleaseData();
		void FlushCache();
		template<typename T>
		void ReleaseBuffer(T* buffer);

		enum ePixelShaderType
		{
			eLIGHTING = 0,
			eUV,
			eNORMAL,
			count
		};
		void SetPixelShaderType(ePixelShaderType type) { mPixelShaderType = type; }
		SINGLETON_METHODS(D3D11MeshRenderer)

	private:
		struct SceneData
		{
			float4x4 mViewProj;
			float4   mEye;
			float4   padding[11];
		};

		struct ObjectData
		{
			float4x4 mWorld;
			float4x4 mWorldInversTrans;
			float4   mColor;
			float4 padding2[7];
		};

		static_assert((sizeof(SceneData) % 256) == 0, "Constant Buffer size must be 256-byte aligned");
		static_assert((sizeof(ObjectData) % 256) == 0, "Constant Buffer size must be 256-byte aligned");
		ID3D11Buffer* mSceneConstantBuffer;
		ID3D11Buffer* mObjectConstantBuffer;

		ePixelShaderType mPixelShaderType;

		ID3D11VertexShader* mVertexShader;
		ID3D11PixelShader* mPixelShaders[ePixelShaderType::count];
		ID3D11InputLayout* mLayout;

		std::map<uint64_t, ID3D11Buffer*> mBuffers;
		struct MeshData
		{
			ID3D11Buffer* mVtxBuffer;
			ID3D11Buffer* mIdxBuffer;
			uint32_t mIndexCount;
			float4x4 mWorld;
			float4x4 mWorldInversTrans;
			float4 mColor;
		};
		std::vector<MeshData> mMeshes;
	};
}
