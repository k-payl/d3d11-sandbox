#include "library.h"

#include "imgui.h"
#include "imgui_impl_win32.h"

extern Data* data;
void CompileShaders();

HWND Window::handle = nullptr;

void Window::Initialize()
{
    const wchar_t* name = L"GameLib";

    HINSTANCE instance = GetModuleHandleW(nullptr);

    WNDCLASSW windowClass = {};
    windowClass.lpfnWndProc = ProceedMessage;
    windowClass.hInstance = instance;
    windowClass.hCursor = (HCURSOR)LoadImageW(nullptr, MAKEINTRESOURCEW(OCR_NORMAL), IMAGE_CURSOR, 0, 0, LR_SHARED);
    windowClass.lpszClassName = name;
    RegisterClassW(&windowClass);

    handle = CreateWindowW(name, name, WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, nullptr, nullptr, instance, nullptr);

    SetSize(1280, 768);

    ShowWindow(handle, SW_SHOWNORMAL);
}

HWND Window::GetHandle()
{
    return handle;
}

DirectX::XMUINT2 Window::GetSize()
{
    RECT clientRect = {};
    GetClientRect(handle, &clientRect);

    return DirectX::XMUINT2(max(clientRect.right - clientRect.left, 1), max(1, clientRect.bottom - clientRect.top));
}

void Window::SetSize(int width, int height)
{
    RECT windowRect = {};
    RECT clientRect = {};
    GetWindowRect(handle, &windowRect);
    GetClientRect(handle, &clientRect);

    int w = (windowRect.right - windowRect.left) - (clientRect.right - clientRect.left) + width;
    int h = (windowRect.bottom - windowRect.top) - (clientRect.bottom - clientRect.top) + height;
    int x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;

    SetWindowPos(handle, nullptr, x, y, w, h, SWP_FRAMECHANGED);
}

bool Window::Update()
{
    MSG message = {};

    while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
    {
        if (message.message == WM_QUIT)
            return false;

        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    return true;
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK Window::ProceedMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_DESTROY)
        PostQuitMessage(0);

    if (ImGui_ImplWin32_WndProcHandler(window, message, wParam, lParam))
        return true;

    if (message == WM_KEYUP && wParam==116) // F5
        CompileShaders();

    if (message == WM_SIZE)
    {
        Graphics::ResizeBackBuffer();

        if (data && data->camera)
            data->camera->ResizeBackBuffer();
    }

    return DefWindowProcW(window, message, wParam, lParam);
}
