#include "D3D11Manager.h"
#include <d3dcompiler.h>
#include <fstream>
#include <assert.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib,  "dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")

namespace Phoenix
{
	bool D3D11Manager::Initialize()
	{
        mWireframe = false;
        mDxgiFactory = nullptr;
        mDevice = nullptr;
        mContext = nullptr;
        mViewport = { 0, 0 };

        mBackBuffer.mTexture = nullptr;
        mBackBuffer.mRTView = nullptr;

        mDepthBuffer.mTexture = nullptr;
        mDepthBuffer.mDSView = nullptr;

        mSwapChain = nullptr;

        // -- Create the Device -- //
        HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&mDxgiFactory));
        if (FAILED(hr))
        {
            return false;
        }

        IDXGIAdapter1* adapter; // adapters are the graphics card (this includes the embedded graphics on the motherboard)

        int adapterIndex = 0; // we'll start looking for directx 11  compatible graphics devices starting at index 0

        bool adapterFound = false; // set this to true when a good one was found

        UINT creationFlags = 0;
#if defined(_DEBUG)
        // If the project is in a debug build, enable the debug layer.
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
        // find first hardware gpu that supports d3d 11
        while (mDxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // we dont want a software device
                adapterIndex++;
                continue;
            }

            // create device
            hr = D3D11CreateDevice(
                adapter,
                D3D_DRIVER_TYPE_UNKNOWN,
                nullptr,
                creationFlags,
                nullptr,
                0,
                D3D11_SDK_VERSION,
                &mDevice,
                nullptr,
                &mContext);
            if (SUCCEEDED(hr))
            {
                adapterFound = true;
                break;
            }
        }

        if (!adapterFound)
        {
            return false;
        }

        //DS States
        {
            D3D11_DEPTH_STENCIL_DESC dsDesc;
            BOOL depthTestEnabled = true;
            dsDesc.DepthEnable = depthTestEnabled;
            dsDesc.DepthWriteMask = depthTestEnabled ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
            dsDesc.DepthFunc = depthTestEnabled ? D3D11_COMPARISON_LESS : D3D11_COMPARISON_ALWAYS;

            // Stencil test parameters
            dsDesc.StencilEnable = false;
            dsDesc.StencilReadMask = 0xFF;
            dsDesc.StencilWriteMask = 0xFF;

            // Stencil operations if pixel is front-facing
            dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
            dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
            dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
            dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

            // Stencil operations if pixel is back-facing
            dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
            dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
            dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
            dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

            // Create depth stencil state
            hr = mDevice->CreateDepthStencilState(&dsDesc, &mDSState);
            if (FAILED(hr))
            {
                return false;
            }
        }

        //Raster state SOLID
        {
            D3D11_RASTERIZER_DESC rasterDesc;
            rasterDesc.AntialiasedLineEnable = false;
            rasterDesc.CullMode = D3D11_CULL_BACK;
            rasterDesc.DepthBias = 0;
            rasterDesc.DepthBiasClamp = 0.0f;
            rasterDesc.DepthClipEnable = true;
            rasterDesc.FillMode = D3D11_FILL_SOLID;
            rasterDesc.FrontCounterClockwise = false;
            rasterDesc.MultisampleEnable = false;
            rasterDesc.ScissorEnable = false;
            rasterDesc.SlopeScaledDepthBias = 0.0f;

            // Create raster state
            hr = mDevice->CreateRasterizerState(&rasterDesc, &mRState);
            if (FAILED(hr))
            {
                return false;
            }
        }

        {
            D3D11_RASTERIZER_DESC rasterDesc;
            rasterDesc.AntialiasedLineEnable = false;
            rasterDesc.CullMode = D3D11_CULL_BACK;
            rasterDesc.DepthBias = 0;
            rasterDesc.DepthBiasClamp = 0.0f;
            rasterDesc.DepthClipEnable = true;
            rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
            rasterDesc.FrontCounterClockwise = false;
            rasterDesc.MultisampleEnable = false;
            rasterDesc.ScissorEnable = false;
            rasterDesc.SlopeScaledDepthBias = 0.0f;

            // Create raster state
            hr = mDevice->CreateRasterizerState(&rasterDesc, &mRStateWireframe);
            if (FAILED(hr))
            {
                return false;
            }
        }
        return true;
	}

    bool D3D11Manager::ResizeSwapChain(const Size& newSize, HWND handle)
    {
        if (mSwapChain != nullptr)
        {
            mBackBuffer.mTexture->Release();//need to release back buffers too
            mBackBuffer.mRTView->Release();
            mDepthBuffer.mTexture->Release();
            mDepthBuffer.mDSView->Release();
            mSwapChain->Release();
            mSwapChain = nullptr;
        }

        // -- Create the Swap Chain (double/tripple buffering) -- //
        DXGI_MODE_DESC backBufferDesc = {}; // this is to describe our display mode
        backBufferDesc.Width = newSize.mWidth; // buffer width
        backBufferDesc.Height = newSize.mHeight; // buffer height
        backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // format of the buffer (rgba 32 bits, 8 bits for each chanel)

        // describe our multi-sampling. We are not multi-sampling, so we set the count to 1 (we need at least one sample of course)
        DXGI_SAMPLE_DESC sampleDesc = {};
        sampleDesc.Count = 1; // multisample count
        sampleDesc.Quality = 0;

        // Describe and create the swap chain.
        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        swapChainDesc.BufferCount = 2; // number of buffers we have
        swapChainDesc.BufferDesc = backBufferDesc; // our back buffer description
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // this says the pipeline will render to this swap chain
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // dxgi will discard the buffer (data) after we call present
        swapChainDesc.OutputWindow = handle; // handle to our window
        swapChainDesc.SampleDesc = sampleDesc; // our multi-sampling description
        swapChainDesc.Windowed = true;

        HRESULT hr = mDxgiFactory->CreateSwapChain(
            mDevice, // the queue will be flushed once the swap chain is created
            &swapChainDesc, // give it the swap chain description we created above
            (IDXGISwapChain**)&mSwapChain // store the created swap chain in a temp IDXGISwapChain interface
        );
        if (FAILED(hr))
        {
            return false;
        }

        // get a pointer directly to the back buffer 
        hr = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&mBackBuffer.mTexture);
        if (FAILED(hr))
        {
            return false;
        }

        // create a render target pointing to the back buffer
        hr = mDevice->CreateRenderTargetView(mBackBuffer.mTexture, nullptr, &mBackBuffer.mRTView);
        if (FAILED(hr))
        {
            return false;
        }

        //depth stencil
        {
            D3D11_TEXTURE2D_DESC desc =
            {
                backBufferDesc.Width, backBufferDesc.Height,
                1, 1,
                DXGI_FORMAT_D32_FLOAT,
                { 1, 0 },
                D3D11_USAGE_DEFAULT,
                D3D11_BIND_DEPTH_STENCIL,
                0, 0
            };
            hr = mDevice->CreateTexture2D(&desc, nullptr, &mDepthBuffer.mTexture);
            if (FAILED(hr))
            {
                return false;
            }

            hr = mDevice->CreateDepthStencilView(mDepthBuffer.mTexture, nullptr, &mDepthBuffer.mDSView);
            if (FAILED(hr))
            {
                return false;
            }
        }

        mViewport.mWidth = newSize.mWidth;
        mViewport.mHeight = newSize.mHeight;
        return true;
    }
    void D3D11Manager::BeginRender()
    {
        float clear_color[] = { 0.45f, 0.45f, 0.45f, 1.00f };
        mContext->OMSetRenderTargets(1, &mBackBuffer.mRTView, mDepthBuffer.mDSView);
        mContext->ClearRenderTargetView(mBackBuffer.mRTView, clear_color);
        mContext->ClearDepthStencilView(mDepthBuffer.mDSView, D3D11_CLEAR_DEPTH, 1.0f, 0);

        D3D11_VIEWPORT viewport{ 0.f, 0.f, (float)mViewport.mWidth, (float)mViewport.mHeight, 0.f, 1.f };
        mContext->RSSetViewports(1, &viewport); // set the viewports
        D3D11_RECT scissorRect{ 0, 0, mViewport.mWidth, mViewport.mHeight };
        mContext->RSSetScissorRects(1, &scissorRect); // set the scissor rects

        mContext->OMSetDepthStencilState(mDSState, 1);
        if(mWireframe)
            mContext->RSSetState(mRStateWireframe);
        else
            mContext->RSSetState(mRState);
    }
    void D3D11Manager::EndRender()
    {
        mSwapChain->Present(0, 0); // Present without vsync
    }

    ID3DBlob* D3D11Manager::CompileShader(const std::vector<byte_t>& fileData, cstr_t srcFile, cstr_t entryPoint, cstr_t profile, const D3D_SHADER_MACRO* defines)
    {
        UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
        flags |= D3DCOMPILE_DEBUG;
#endif
        if (defines == nullptr)
        {
            const D3D_SHADER_MACRO macros[] =
            {
                  NULL, NULL
            };
            defines = macros;
        }

        ID3DBlob* shaderBlob = nullptr;
        ID3DBlob* errorBlob = nullptr;
        HRESULT hr = D3DCompile(fileData.data(), fileData.size(), srcFile, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
            entryPoint, profile,
            flags, 0, &shaderBlob, &errorBlob);
        if (FAILED(hr))
        {
            if (errorBlob)
            {
                OutputDebugStringA((char*)errorBlob->GetBufferPointer());
                errorBlob->Release();
            }

            if (shaderBlob)
            {
                shaderBlob->Release();
                shaderBlob = nullptr;
            }
        }

        return shaderBlob;
    }

	void D3D11Manager::Shutdown()
	{
        if (mBackBuffer.mRTView != nullptr)
            mBackBuffer.mRTView->Release();
        if (mBackBuffer.mTexture != nullptr)
            mBackBuffer.mTexture->Release();

        if (mDevice != nullptr)
            mDevice->Release();
        if (mContext != nullptr)
            mContext->Release();
        if (mDxgiFactory != nullptr)
            mDxgiFactory->Release();
        if (mSwapChain != nullptr)
            mSwapChain->Release();

        mRState->Release();
        mRStateWireframe->Release();
        mDSState->Release();

#if defined(_DEBUG)
        IDXGIDebug1* pDebug = NULL;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
        {
            pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
            pDebug->Release();
        }
#endif
	}
}
