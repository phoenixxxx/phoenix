#include "D3D11MeshRenderer.h"
#include <Utils/Utils.h>

namespace Phoenix
{
    bool D3D11MeshRenderer::Initialize()
    {
        mPixelShaderType = ePixelShaderType::eLIGHTING;
        mVertexShader = nullptr;
        std::memset(mPixelShaders, 0, sizeof(mPixelShaders));
        mLayout = nullptr;

        ReleaseData();

        HRESULT hr;
        cstr_t shaderPath = "Assets/Shaders/Triangles.hlsl";
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
                {"NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0}
            };
            // create the input layout
            hr = Phoenix::D3D11Manager::Instance()->GetDevice()->CreateInputLayout(inputElemDesc, ARRAYSIZE(inputElemDesc), blob->GetBufferPointer(), blob->GetBufferSize(), &mLayout);
            blob->Release();
            if (FAILED(hr))
            {
                return false;
            }

            //pixel shader
            {
                blob = Phoenix::D3D11Manager::Instance()->CompileShader(shaderCode, shaderPath, "ps_main", "ps_5_0");
                hr = Phoenix::D3D11Manager::Instance()->GetDevice()->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &mPixelShaders[eLIGHTING]);
                if (FAILED(hr))
                {
                    return false;
                }
                blob->Release();
            }
            {
                const D3D_SHADER_MACRO defUVs[] =
                {
                    "RENDER_UVS", "1",
                    NULL, NULL
                };
                blob = Phoenix::D3D11Manager::Instance()->CompileShader(shaderCode, shaderPath, "ps_main", "ps_5_0", defUVs);
                hr = Phoenix::D3D11Manager::Instance()->GetDevice()->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &mPixelShaders[eUV]);
                if (FAILED(hr))
                {
                    return false;
                }
                blob->Release();
            }
            {
                const D3D_SHADER_MACRO defNmls[] =
                {
                    "RENDER_NORMALS", "1",
                    NULL, NULL
                };
                blob = Phoenix::D3D11Manager::Instance()->CompileShader(shaderCode, shaderPath, "ps_main", "ps_5_0", defNmls);
                hr = Phoenix::D3D11Manager::Instance()->GetDevice()->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &mPixelShaders[eNORMAL]);
                if (FAILED(hr))
                {
                    return false;
                }
                blob->Release();
            }
        }

        // constant buffer Scene
        {
            D3D11_BUFFER_DESC bd;
            ZeroMemory(&bd, sizeof(bd));
            bd.Usage = D3D11_USAGE_DYNAMIC;
            bd.ByteWidth = sizeof(SceneData);
            bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            hr = Phoenix::D3D11Manager::Instance()->GetDevice()->CreateBuffer(&bd, nullptr, &mSceneConstantBuffer);
            if (FAILED(hr))
            {
                return false;
            }
        }

        // constant buffer Object
        {
            D3D11_BUFFER_DESC bd;
            ZeroMemory(&bd, sizeof(bd));
            bd.Usage = D3D11_USAGE_DYNAMIC;
            bd.ByteWidth = sizeof(ObjectData);
            bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            hr = Phoenix::D3D11Manager::Instance()->GetDevice()->CreateBuffer(&bd, nullptr, &mObjectConstantBuffer);
            if (FAILED(hr))
            {
                return false;
            }
        }
        return true;
    }
    void D3D11MeshRenderer::Shutdown()
    {
        ReleaseData();
    }

    void D3D11MeshRenderer::Render(const Camera& camera)
    {
        if (mMeshes.size() == 0)
            return;

        //upload constant data Scene
        {
            HRESULT hr;
            D3D11_MAPPED_SUBRESOURCE mapped;
            hr = Phoenix::D3D11Manager::Instance()->GetContext()->Map(mSceneConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
            if (FAILED(hr))
                return;
            SceneData scene;
            scene.mViewProj = camera.GetViewProjectionMatrix();
            scene.mEye = camera.GetEyePosition();
            std::memcpy(mapped.pData, &scene, sizeof(SceneData));
            Phoenix::D3D11Manager::Instance()->GetContext()->Unmap(mSceneConstantBuffer, 0);
            D3D11Manager::Instance()->GetContext()->VSSetConstantBuffers(0, 1, &mSceneConstantBuffer);
            D3D11Manager::Instance()->GetContext()->PSSetConstantBuffers(0, 1, &mSceneConstantBuffer);
        }

        for (auto& mesh : mMeshes)
        {
            //upload object data Scene
            HRESULT hr;
            D3D11_MAPPED_SUBRESOURCE mapped;
            hr = Phoenix::D3D11Manager::Instance()->GetContext()->Map(mObjectConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
            if (FAILED(hr))
                return;
            ObjectData object;
            object.mWorld = mesh.mWorld; 
            object.mWorldInversTrans = mesh.mWorldInversTrans;
            object.mColor = mesh.mColor;
            std::memcpy(mapped.pData, &object, sizeof(ObjectData));
            Phoenix::D3D11Manager::Instance()->GetContext()->Unmap(mObjectConstantBuffer, 0);
            D3D11Manager::Instance()->GetContext()->VSSetConstantBuffers(1, 1, &mObjectConstantBuffer);
            D3D11Manager::Instance()->GetContext()->PSSetConstantBuffers(1, 1, &mObjectConstantBuffer);

            //Draw
            UINT vertexStride = sizeof(Vertex);
            UINT vertexOffset = 0;
            D3D11Manager::Instance()->GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            D3D11Manager::Instance()->GetContext()->IASetInputLayout(mLayout);
            D3D11Manager::Instance()->GetContext()->IASetVertexBuffers(
                0,
                1,
                &mesh.mVtxBuffer,
                &vertexStride,
                &vertexOffset);
            D3D11Manager::Instance()->GetContext()->IASetIndexBuffer(
                mesh.mIdxBuffer,
                DXGI_FORMAT_R32_UINT,
                0);

            D3D11Manager::Instance()->GetContext()->VSSetShader(mVertexShader, NULL, 0);
            D3D11Manager::Instance()->GetContext()->PSSetShader(mPixelShaders[mPixelShaderType], NULL, 0);
            D3D11Manager::Instance()->GetContext()->DrawIndexed(mesh.mIndexCount, 0, 0);
        }

        mMeshes.clear();
    }
    void D3D11MeshRenderer::ReleaseData()
    {
        if (mVertexShader != nullptr)
        {
            mVertexShader->Release();
            mVertexShader = nullptr;
        }
        if (mLayout != nullptr)
        {
            mLayout->Release();
            mLayout = nullptr;
        }
        if (mSceneConstantBuffer != nullptr)
        {
            mSceneConstantBuffer->Release();
            mSceneConstantBuffer = nullptr;
        }
        if (mObjectConstantBuffer != nullptr)
        {
            mObjectConstantBuffer->Release();
            mObjectConstantBuffer = nullptr;
        }
        for (auto pxl : mPixelShaders)
        {
            if(pxl != nullptr)
                pxl->Release();
        }
    }
    void D3D11MeshRenderer::FlushCache()
    {
        for (auto buffer : mBuffers)
        {
            buffer.second->Release();
        }
        mBuffers.clear();
    }
    template<typename T>
    void D3D11MeshRenderer::ReleaseBuffer(T* buffer)
    {
        auto vtxIter = mBuffers.find(reinterpret_cast<uint64_t>(buffer));
        if (vtxIter != mBuffers.end())
        {
            vtxIter->second->Release();
            mBuffers.erase(vtxIter);
        }
    }
    void D3D11MeshRenderer::DrawMesh(Vertex* vertexBuffer, size_t vtxCount, int3* indexBuffer, size_t faceCount, const float4x4& world, const float4x4& worldInversTrans, const float4& color)
    {
        //check if we have the vertex buffer
        HRESULT hr;
        ID3D11Buffer* d3dVtxBuffer;
        auto vtxIter = mBuffers.find(reinterpret_cast<uint64_t>(vertexBuffer));
        if (vtxIter == mBuffers.end())
        {
            D3D11_BUFFER_DESC bd;
            ZeroMemory(&bd, sizeof(bd));
            bd.Usage = D3D11_USAGE_IMMUTABLE;
            bd.ByteWidth = vtxCount * sizeof(Vertex);
            bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            bd.CPUAccessFlags = 0;

            D3D11_SUBRESOURCE_DATA data =
            {
                vertexBuffer,
                bd.ByteWidth,
                bd.ByteWidth
            };
            hr = D3D11Manager::Instance()->GetDevice()->CreateBuffer(&bd, &data, &d3dVtxBuffer);
            if (FAILED(hr))
            {
                return;
            }

            mBuffers[reinterpret_cast<uint64_t>(vertexBuffer)] = d3dVtxBuffer;
        }
        else
            d3dVtxBuffer = vtxIter->second;

        //Check if we have Index buffer
        ID3D11Buffer* d3dIdxBuffer;
        auto idxIter = mBuffers.find(reinterpret_cast<uint64_t>(indexBuffer));
        if (idxIter == mBuffers.end())
        {
            D3D11_BUFFER_DESC bd;
            ZeroMemory(&bd, sizeof(bd));
            bd.Usage = D3D11_USAGE_IMMUTABLE;
            bd.ByteWidth = faceCount * sizeof(int3);
            bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
            bd.CPUAccessFlags = 0;

            D3D11_SUBRESOURCE_DATA data =
            {
                indexBuffer,
                bd.ByteWidth,
                bd.ByteWidth
            };
            hr = D3D11Manager::Instance()->GetDevice()->CreateBuffer(&bd, &data, &d3dIdxBuffer);
            if (FAILED(hr))
            {
                return;
            }

            mBuffers[reinterpret_cast<uint64_t>(indexBuffer)] = d3dIdxBuffer;
        }
        else
            d3dIdxBuffer = idxIter->second;

        MeshData mesh = {
            d3dVtxBuffer,
            d3dIdxBuffer,
            faceCount * 3,//3 indices per face
            world,
            worldInversTrans,
            color
        };
        mMeshes.push_back(mesh);
    }

    void D3D11MeshRenderer::DrawTriangle(const float3& p0, const float3& p1, const float3& p2, const float3& color)
    {

    }
}