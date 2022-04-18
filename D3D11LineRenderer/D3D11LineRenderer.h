#pragma once

#include <vector>
#include <Utils/Math.h>
#include <Utils/VectorMath.h>
#include <Utils/Types.h>
#include <D3D11/D3D11Manager.h>
#include <Camera/Camera.h>
#include "Utils/Singleton.h"

namespace Phoenix
{
	class D3D11LineRenderer
	{
	public:
		bool Initialize();
		void Shutdown();
		void Render(const Camera& camera);

		void DrawSphere(const float3& p, const float3& color, float radius);
		void DrawCircle(const float3& p, const float3& n, const float3& color, float radius);
		void DrawCircle(const float3& p, const float3& u, const float3& v, const float3& color, float radius);
		void DrawNormal(const float3& p, const float3& n, const float3& color, const Camera& camera);
		void DrawCone(const float3& vertex, const float3& center, const float3& color, float radius);
		void DrawConeAngle(const float3& vertex, const float3& center, const float3& color, float angle);

		void DrawBox(const float3& min, const float3& max, const float3& color);
		void DrawPlane(const float3& p, const float3& n, const float3& color, float scale);
		void DrawPoint(const float3& p, const float3& color, const Camera& camera);
		void DrawLine(const float3& p0, const float3& p1, const float3& color);
		

	private:
		bool UploadVertexData();
		void ReleaseData();

	private:
		ID3D11VertexShader* mVertexShader;
		ID3D11PixelShader*  mPixelShader;
		ID3D11InputLayout* mLayout;

	private:
		struct Vertex
		{
			float4 mPosition;
			float4 mColor;
		};
		std::vector<Vertex> mVertices;

		ID3D11Buffer* mVertexBuffer;
		size_t mCurrentCapacity;

		struct GlobalData
		{
			float4x4 mViewProjection;
			float padding[48]; // Padding so the constant buffer is 256-byte aligned.
		};
		static_assert((sizeof(GlobalData) % 256) == 0, "Constant Buffer size must be 256-byte aligned");
		ID3D11Buffer* mGlobalConstantBuffer;

		SINGLETON_METHODS(D3D11LineRenderer)
	};
}
