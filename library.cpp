#include "library.h"

void InitializeLibrary()
{
    static bool isInitialized = false;

    if (!isInitialized)
    {
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        Window::Initialize();
        Graphics::Initialize();
        InitUI(Window::GetHandle(), &Graphics::GetContext(), &Graphics::GetDevice());

        isInitialized = true;
    }
}

bool UpdateLibrary()
{
    Graphics::Update();
    BeginFrameUI();
    return Window::Update();
}