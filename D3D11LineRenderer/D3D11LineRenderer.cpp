#include "D3D11LineRenderer.h"
#include <Utils/Utils.h>

namespace Phoenix
{
	bool D3D11LineRenderer::Initialize()
	{
        mVertexShader = nullptr;
        mPixelShader = nullptr;
        mLayout = nullptr;
        mVertexBuffer = nullptr;
        mGlobalConstantBuffer = nullptr;
        mCurrentCapacity = 0;

        ReleaseData();

        HRESULT hr;
        cstr_t shaderPath = "Assets/Shaders/Simple.hlsl";
        std::vector<byte_t> shaderCode = LoadFile(shaderPath);
        if (shaderCode.size() != 0)
        {
            ID3DBlob* blob;
            //vertex shader
            blob = Phoenix::D3D11Manager::Instance()->CompileShader(shaderCode, shaderPath, "vs_main", "vs_5_0");
            hr = Phoenix::D3D11Manager::Instance()->GetDevice()->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &mVertexShader);
            if (FAILED(hr))
            {
                return false;
            }
            D3D11_INPUT_ELEMENT_DESC inputElemDesc[] = 
            {
                {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0}
            };
            // create the input layout
            hr = Phoenix::D3D11Manager::Instance()->GetDevice()->CreateInputLayout(inputElemDesc, ARRAYSIZE(inputElemDesc), blob->GetBufferPointer(), blob->GetBufferSize(), &mLayout);
            blob->Release();
            if (FAILED(hr))
            {
                return false;
            }

            //pixel shader
            blob = Phoenix::D3D11Manager::Instance()->CompileShader(shaderCode, shaderPath, "ps_main", "ps_5_0");
            hr = Phoenix::D3D11Manager::Instance()->GetDevice()->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &mPixelShader);
            blob->Release();
            if (FAILED(hr))
            {
                return false;
            }
        }

        // constant buffer
        {
            D3D11_BUFFER_DESC bd;
            ZeroMemory(&bd, sizeof(bd));
            bd.Usage = D3D11_USAGE_DYNAMIC;
            bd.ByteWidth = sizeof(GlobalData);
            bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            hr = Phoenix::D3D11Manager::Instance()->GetDevice()->CreateBuffer(&bd, nullptr, &mGlobalConstantBuffer);
            if (FAILED(hr))
            {
                return false;
            }
        }

        return true;
	}
	void D3D11LineRenderer::Shutdown()
	{
        ReleaseData();
	}

    void D3D11LineRenderer::ReleaseData()
    {
        if (mVertexShader != nullptr)
        {
            mVertexShader->Release();
            mVertexShader = nullptr;
        }
        if (mPixelShader != nullptr)
        {
            mPixelShader->Release();
            mPixelShader = nullptr;
        }
        if (mLayout != nullptr)
        {
            mLayout->Release();
            mLayout = nullptr;
        }

        if (mVertexBuffer != nullptr)
        {
            mVertexBuffer->Release();
            mVertexBuffer = nullptr;
            mCurrentCapacity = 0;
        }

        if (mGlobalConstantBuffer != nullptr)
        {
            mGlobalConstantBuffer->Release();
            mGlobalConstantBuffer = nullptr;
        }

    }
    void D3D11LineRenderer::DrawBox(const float3& min, const float3& max, const float3& color)
    {
        const float3 delta = max - min;
        const float3 min1 = min + float3(delta.x, 0, 0);
        const float3 min2 = min + float3(delta.x, 0, delta.z);
        const float3 min3 = min + float3(0, 0, delta.z);

        const float3 max1 = max - float3(delta.x, 0, 0);
        const float3 max2 = max - float3(delta.x, 0, delta.z);
        const float3 max3 = max - float3(0, 0, delta.z);

        DrawLine(min,  min1, color);
        DrawLine(min1, min2, color);
        DrawLine(min2, min3, color);
        DrawLine(min3, min, color);

        DrawLine(max,  max1, color);
        DrawLine(max1, max2, color);
        DrawLine(max2, max3, color);
        DrawLine(max3, max, color);

        DrawLine(min3,  max1, color);
        DrawLine(min, max2, color);
        DrawLine(min1, max3, color);
        DrawLine(min2, max, color);
    }

    void D3D11LineRenderer::DrawSphere(const float3& p, const float3& color, float radius)
    {
		DrawCircle(p, float3(1,0,0), color, radius);
		DrawCircle(p, float3(0, 1, 0), color, radius);
		DrawCircle(p, float3(0, 0, 1), color, radius);
    }
    
    void D3D11LineRenderer::DrawCircle(const float3& p, const float3& n, const float3& color, float radius)
    {
		float3 u = float3::orthogonal(n);
		float3 v = float3::normalize(float3::cross(u, n));
        DrawCircle(p, u, v, color, radius);
    }

    void D3D11LineRenderer::DrawCircle(const float3& p, const float3& u, const float3& v, const float3& color, float radius)
    {
        int resolution = 40;
        float delta = TwoPi / resolution;
        float theta = 0;
        float prevx(radius), prevy(0);
        for (int i = 0; i < resolution; ++i)
        {
            theta += delta;
            float x = radius * cosf(theta);
            float y = radius * sinf(theta);

            DrawLine(p + (prevx * u + prevy * v), p + (x * u + y * v), color);

            prevx = x;
            prevy = y;
        }
    }

    void D3D11LineRenderer::DrawNormal(const float3& p, const float3& n, const float3& color, const Camera& camera)
    {
        static const uint32_t nSz = 20;

        const auto& vp = Phoenix::D3D11Manager::Instance()->GetViewport();
        float viewPlaneW = camera.GetViewPlaneWidth();

        const float3 viewP = camera.WorldToViewPoint(p);
        float offset = (nSz * viewPlaneW / vp.mWidth) * viewP.y;

        DrawLine(p, p + offset*n, color);
    }

    void D3D11LineRenderer::DrawConeAngle(const float3& vertex, const float3& center, const float3& color, float angle)
    {
        float l = (center - vertex).length();
        DrawCone(vertex, center, color, l * tanf(angle));
    }

    void D3D11LineRenderer::DrawCone(const float3& vertex, const float3& center, const float3& color, float radius)
    {
        const float3& n = float3::normalize(vertex - center);
        float3 u = float3::orthogonal(n);
        float3 v = float3::normalize(float3::cross(u, n));
        DrawCircle(center, u, v, color, radius);

		DrawLine(center + radius * u, vertex, color);
		DrawLine(center - radius * u, vertex, color);
        DrawLine(center + radius * v, vertex, color);
        DrawLine(center - radius * v, vertex, color);
    }

    void D3D11LineRenderer::DrawPlane(const float3& p, const float3& n, const float3& color, float scale)
    {
        float3 u = float3::orthogonal(n);
        float3 v = float3::normalize(float3::cross(u, n));

        float halfScale = 0.5f * scale;

        const float3 p0 = p - halfScale * u + halfScale * v;
        const float3 p1 = p + halfScale * u + halfScale * v;
        const float3 p2 = p - halfScale * u - halfScale * v;
        const float3 p3 = p + halfScale * u - halfScale * v;

        DrawLine(p0, p1, color);
        DrawLine(p2, p3, color);
        DrawLine(p0, p2, color);
        DrawLine(p1, p3, color);

        DrawLine(p, p + scale * n, color);
    }

    void D3D11LineRenderer::DrawPoint(const float3& p, const float3& color, const Camera& camera)
    {
        static const uint32_t ptSz = 5;

        const auto& vp = Phoenix::D3D11Manager::Instance()->GetViewport();
        float viewPlaneW = camera.GetViewPlaneWidth();

        const float3 viewP = camera.WorldToViewPoint(p);
        float offset = (ptSz * viewPlaneW / vp.mWidth) * viewP.y;

		DrawLine(float3(p.x - offset, p.y, p.z), float3(p.x + offset, p.y, p.z), color);
		DrawLine(float3(p.x, p.y - offset, p.z), float3(p.x, p.y + offset, p.z), color);
		DrawLine(float3(p.x, p.y, p.z - offset), float3(p.x, p.y, p.z + offset), color);
    }

    void D3D11LineRenderer::DrawLine(const float3& p0, const float3& p1, const float3& color)
    {
        Vertex v0, v1;
        v0.mPosition = float4(p0.x, p0.y, p0.z, 1);
        v0.mColor  = float4(color.x, color.y, color.z, 1);
        mVertices.push_back(v0);

        v1.mPosition = float4(p1.x, p1.y, p1.z, 1);
        v1.mColor = float4(color.x, color.y, color.z, 1);
        mVertices.push_back(v1);
    }

    bool D3D11LineRenderer::UploadVertexData()
    {
        HRESULT hr;
		//vertex buffer
        if(mVertices.size() >= mCurrentCapacity)
		{
            //release vtx buffer
            if (mVertexBuffer != nullptr)
                mVertexBuffer->Release();

            mCurrentCapacity = mVertices.size();// std::max(128LLU, mCurrentCapacity * 2);

			D3D11_BUFFER_DESC bd;
			ZeroMemory(&bd, sizeof(bd));
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.ByteWidth = mCurrentCapacity * sizeof(Vertex);
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			hr = D3D11Manager::Instance()->GetDevice()->CreateBuffer(&bd, nullptr, &mVertexBuffer);
			if (FAILED(hr))
			{
				return false;
			}
		}

        D3D11_MAPPED_SUBRESOURCE mapped;
        hr = Phoenix::D3D11Manager::Instance()->GetContext()->Map(mVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        if (FAILED(hr))
        {
            return false;
        }
        std::memcpy(mapped.pData, mVertices.data(), mVertices.size() * sizeof(Vertex));
        Phoenix::D3D11Manager::Instance()->GetContext()->Unmap(mVertexBuffer, 0);
        return true;
    }

    void D3D11LineRenderer::Render(const Camera& camera)
    {
        if (mVertices.size() == 0)
            return;

        bool uploaded = UploadVertexData();
        if (uploaded)
        {
            //upload constant data
            HRESULT hr;
            D3D11_MAPPED_SUBRESOURCE mapped;
            hr = Phoenix::D3D11Manager::Instance()->GetContext()->Map(mGlobalConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
            if (FAILED(hr))
                return;
            GlobalData global;
            global.mViewProjection = camera.GetViewProjectionMatrix();
            std::memcpy(mapped.pData, &global, sizeof(GlobalData));
            Phoenix::D3D11Manager::Instance()->GetContext()->Unmap(mGlobalConstantBuffer, 0);
            D3D11Manager::Instance()->GetContext()->VSSetConstantBuffers(0, 1, &mGlobalConstantBuffer);

            //Draw
            UINT vertexStride = sizeof(Vertex);
            UINT vertexOffset = 0;
            D3D11Manager::Instance()->GetContext()->IASetPrimitiveTopology(
                D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
            D3D11Manager::Instance()->GetContext()->IASetInputLayout(mLayout);
            D3D11Manager::Instance()->GetContext()->IASetVertexBuffers(
                0,
                1,
                &mVertexBuffer,
                &vertexStride,
                &vertexOffset);

            D3D11Manager::Instance()->GetContext()->VSSetShader(mVertexShader, NULL, 0);
            D3D11Manager::Instance()->GetContext()->PSSetShader(mPixelShader, NULL, 0);
            D3D11Manager::Instance()->GetContext()->Draw(mVertices.size(), 0);
        }

        mVertices.clear();
    }
}