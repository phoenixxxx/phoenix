#include "Window.h"

#ifndef IMGUI_IMPL_API
#define IMGUI_IMPL_API
#endif // !IMGUI_IMPL_API
#include <ThirdParty/imgui/imgui.h> 
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


namespace Phoenix
{
    LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
            return true;

        static bool dragOn;
        static uint32_t prevX, prevY;
        static Window* window = nullptr;
        switch (msg)
        {
        case WM_MBUTTONUP:
        {
            if (!dragOn)
            {
                Phoenix::MouseInfo mouse{ 0 };
                prevX = mouse.x = GET_X_LPARAM(lParam);
                prevY = mouse.y = GET_Y_LPARAM(lParam);
                mouse.mButton = MouseButton::eMBCenter;
                if (MK_SHIFT & wParam)
                {
                    mouse.mModifier |= SystemKeyBits::eSKShift;
                }
                if (MK_CONTROL & wParam)
                {
                    mouse.mModifier |= SystemKeyBits::eSKControl;
                }
                window->mClickEvent(mouse);
            }
            dragOn = false;
            break;
        }
        case WM_LBUTTONUP:
        {
            if (!dragOn)
            {
                Phoenix::MouseInfo mouse{ 0 };
                prevX = mouse.x = GET_X_LPARAM(lParam);
                prevY = mouse.y = GET_Y_LPARAM(lParam);
                mouse.mButton = MouseButton::eMBLeft;
                if (MK_SHIFT & wParam)
                {
                    mouse.mModifier |= SystemKeyBits::eSKShift;
                }
                if (MK_CONTROL & wParam)
                {
                    mouse.mModifier |= SystemKeyBits::eSKControl;
                }
                window->mClickEvent(mouse);
            }
            dragOn = false;
            break;
        }
        case WM_RBUTTONUP:
        {
            if (!dragOn)
            {
                Phoenix::MouseInfo mouse { 0 };
                prevX = mouse.x = GET_X_LPARAM(lParam);
                prevY = mouse.y = GET_Y_LPARAM(lParam);
                mouse.mButton = MouseButton::eMBRight;
                if (MK_SHIFT & wParam)
                {
                    mouse.mModifier |= SystemKeyBits::eSKShift;
                }
                if (MK_CONTROL & wParam)
                {
                    mouse.mModifier |= SystemKeyBits::eSKControl;
                }
                window->mClickEvent(mouse);
            }
            dragOn = false;
            break;
        }

        case WM_MBUTTONDOWN:
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        {
            Phoenix::MouseInfo mouse{ 0 };
            prevX = mouse.x = GET_X_LPARAM(lParam);
            prevY = mouse.y = GET_Y_LPARAM(lParam);
            break;
        }

        case WM_MOUSEMOVE:
        {
            Phoenix::MouseInfo mouse{0};
            if (MK_RBUTTON & wParam)
                mouse.mButton = MouseButton::eMBRight;
            else if (MK_LBUTTON & wParam)
                mouse.mButton = MouseButton::eMBLeft;
            else if (MK_MBUTTON & wParam)
                mouse.mButton = MouseButton::eMBCenter;
            if (mouse.mButton != MouseButton::eMBNone)
            {
                mouse.x = GET_X_LPARAM(lParam);
                mouse.y = GET_Y_LPARAM(lParam);
                mouse.dx = mouse.x - prevX;
                mouse.dy = mouse.y - prevY;
                prevX = mouse.x;
                prevY = mouse.y;
                if (MK_SHIFT & wParam)
                {
                    mouse.mModifier |= SystemKeyBits::eSKShift;
                }
                if (MK_CONTROL & wParam)
                {
                    mouse.mModifier |= SystemKeyBits::eSKControl;
                }
                dragOn = true;
                window->mDragEvent(mouse);
            }
            else
            {
                mouse.x = GET_X_LPARAM(lParam);
                mouse.y = GET_Y_LPARAM(lParam);
                mouse.dx = 0;
                mouse.dy = 0;
                window->mMouseMoveEvent(mouse);
            }
            
            break;
        }
        case WM_MOUSEWHEEL:
        {
            Phoenix::MouseInfo mouse{ 0 };
            mouse.dz = GET_WHEEL_DELTA_WPARAM(wParam);
            if (MK_SHIFT & wParam)
            {
                mouse.mModifier |= SystemKeyBits::eSKShift;
            }
            if (MK_CONTROL & wParam)
            {
                mouse.mModifier |= SystemKeyBits::eSKControl;
            }
            window->mMouseWheelEvent(mouse);
            break;
        }

        case WM_SIZE:
            if (wParam != SIZE_MINIMIZED)
            {
                uint32_t width  = (uint32_t)LOWORD(lParam);
                uint32_t height = (uint32_t)HIWORD(lParam);
                window->mCurrenSize.mWidth = width;
                window->mCurrenSize.mHeight = height;
                window->mResizeEvent(width, height);
            }
            break;
        case WM_USER:
            if (window == nullptr)
                window = reinterpret_cast<Window*>(lParam);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }
        return DefWindowProc(hwnd,
            msg,
            wParam,
            lParam);
    }

    Window::Window()
    {

    }

    Window::~Window()
    {

    }

    Window* Window::Instance()
    {
        static Window instance;
        return &instance;
    }

	bool Window::Initialize(HINSTANCE hInstance, cstr_t title, uint32_t width, uint32_t height)//, ResizeMethod_t resize, 
        //MouseMethod_t click, MouseMethod_t drag, MouseMethod_t wheel)
	{
        static cstr_t windowName = "PhoenixWindow";
        //if (fullscreen)
        //{
        //    HMONITOR hmon = MonitorFromWindow(hwnd,
        //        MONITOR_DEFAULTTONEAREST);
        //    MONITORINFO mi = { sizeof(mi) };
        //    GetMonitorInfo(hmon, &mi);

        //    width = mi.rcMonitor.right - mi.rcMonitor.left;
        //    height = mi.rcMonitor.bottom - mi.rcMonitor.top;
        //}
        WNDCLASSEX wc;

        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = WndProc;
        wc.cbClsExtra = NULL;
        wc.cbWndExtra = NULL;
        wc.hInstance = hInstance;
        wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
        wc.lpszMenuName = NULL;
        wc.lpszClassName = windowName;
        wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

        if (!RegisterClassEx(&wc))
        {
            MessageBox(NULL, "Error registering class", "Error", MB_OK | MB_ICONERROR);
            return false;
        }

        RECT rect {0, 0, (LONG)width, (LONG)height};
        AdjustWindowRect(
            &rect,
            WS_OVERLAPPEDWINDOW,
            false
        );
        uint32_t wWidth = rect.right - rect.left;
        uint32_t wHeight = rect.bottom - rect.top;
        mHandle = CreateWindowEx(NULL,
            windowName,
            title,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            wWidth, wHeight,
            NULL,
            NULL,
            hInstance,
            NULL);
        if (!mHandle)
        {
            MessageBox(NULL, "Error creating window", "Error", MB_OK | MB_ICONERROR);
            return false;
        }

        mCurrenSize.mWidth = width;
        mCurrenSize.mHeight = height;

        //mResizeMethod = resize;
        //mClickMethod = click;
        //mDragMethod = drag;
        //mMouseWheelMethod = wheel;

        //send the window's this object
        SendMessage(mHandle, WM_USER, 0, (WPARAM)this);

        return true;
	}

    void Window::Show(int ShowWnd)
    {
        ShowWindow(mHandle, ShowWnd);
        UpdateWindow(mHandle);
    }
}
