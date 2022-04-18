#pragma once
#include <Utils/Singleton.h>
#include <Utils/Types.h>

#include <dxgi1_6.h>
#include <d3d11.h>

//#define D3D12DEBUG
#if defined(_DEBUG)
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif

#include <Window/Window.h>

namespace Phoenix
{
	class D3D11Manager
	{
	public:
		bool Initialize();
		void Shutdown();
		bool ResizeSwapChain(const Size& newSize, HWND handle);
		const Size& GetViewport() { return mViewport; }

		void BeginRender();
		void EndRender();

	public:
		ID3D11Device* GetDevice() { return mDevice; }
		ID3D11DeviceContext* GetContext() { return mContext; }
		ID3DBlob* CompileShader(const std::vector<byte_t>& fileData, cstr_t srcFile, cstr_t entryPoint, cstr_t profile, const D3D_SHADER_MACRO* defines = nullptr);
		void SetWireframe(bool wire) { mWireframe = wire; }
	private:
		Size mViewport;

	private:
		IDXGIFactory7* mDxgiFactory;
		ID3D11Device* mDevice;
		ID3D11DeviceContext* mContext;
		IDXGISwapChain3* mSwapChain;

		struct
		{
			ID3D11Texture2D* mTexture;
			ID3D11RenderTargetView* mRTView;
		}mBackBuffer;

		struct
		{
			ID3D11Texture2D* mTexture;
			ID3D11DepthStencilView* mDSView;
		}mDepthBuffer;

		ID3D11DepthStencilState* mDSState;
		ID3D11RasterizerState* mRState, *mRStateWireframe;
		bool mWireframe;

		SINGLETON_METHODS(D3D11Manager)
	};
}
