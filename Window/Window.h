#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <Windowsx.h>
#include <commdlg.h>
#include <memory>
#include <functional>
#include <Utils/Types.h>
#include <Utils/Input.h>
#include <Utils/Event.h>

namespace Phoenix
{	
	class Window
	{
	public:
		typedef Event<uint32_t, uint32_t> ResizeEvent_t;
		typedef Event<const MouseInfo&>   MouseEvent_t;

	private:
		Window();
	public:
		~Window();
		static Window* Instance();
	public:
		bool Initialize(HINSTANCE hInstance, cstr_t title, uint32_t width, uint32_t height);//, ResizeMethod_t resize, MouseMethod_t click, MouseMethod_t drag, MouseMethod_t wheel);
		void Show(int ShowWnd);

		const Size& GetCurrenSize() { return mCurrenSize; }
		HWND  GetWindowHandle() { return mHandle; }

	private:
		static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	private:
		HWND mHandle;
		Size mCurrenSize;

	public:
		ResizeEvent_t mResizeEvent;
		MouseEvent_t mClickEvent;
		MouseEvent_t mDragEvent;
		MouseEvent_t mMouseMoveEvent;
		MouseEvent_t mMouseWheelEvent;
	}; 
}