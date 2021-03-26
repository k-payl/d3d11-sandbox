#include "library.h"

extern Data* data;

Microsoft::WRL::ComPtr<ID3D11Device> Graphics::device = nullptr;
Microsoft::WRL::ComPtr<ID3D11DeviceContext> Graphics::context = nullptr;
Microsoft::WRL::ComPtr<IDXGISwapChain> Graphics::swapChain = nullptr;
Microsoft::WRL::ComPtr<IWICImagingFactory> Graphics::imageFactory = nullptr;

void Graphics::ResizeBackBuffer()
{
    if (!device)
        return;

    HRESULT hr = E_FAIL;

    Microsoft::WRL::ComPtr<IDXGIFactory1> dxgiFactory;
    {
        Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
        hr = device.As(&dxgiDevice);
        if (SUCCEEDED(hr))
        {
            Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
            hr = dxgiDevice->GetAdapter(&adapter);
            if (SUCCEEDED(hr))
                hr = adapter->GetParent(__uuidof(IDXGIFactory1), &dxgiFactory);
        }
    }

    ThrowIfFailed(hr);

    Microsoft::WRL::ComPtr<IDXGIFactory> dxgiFactory2;
    hr = dxgiFactory.As(&dxgiFactory2);

    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferDesc.Width = Window::GetSize().x;
    sd.BufferDesc.Height = Window::GetSize().y;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 1;
    sd.OutputWindow = Window::GetHandle();
    sd.Windowed = true;

    hr = dxgiFactory->CreateSwapChain(&GetDevice(), &sd, &swapChain);
    dxgiFactory->MakeWindowAssociation(Window::GetHandle(), DXGI_MWA_NO_ALT_ENTER);

    if (!swapChain)
        return;

    D3D11_VIEWPORT viewport = {};
    viewport.Width = (float)Window::GetSize().x;
    viewport.Height = (float)Window::GetSize().y;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    context->RSSetViewports(1, &viewport);
}

void Graphics::Initialize()
{
    std::vector<D3D_DRIVER_TYPE> driverTypes
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
        D3D_DRIVER_TYPE_SOFTWARE,
    };

    CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, reinterpret_cast<void**>(imageFactory.GetAddressOf()));

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferDesc.Width = Window::GetSize().x;
    swapChainDesc.BufferDesc.Height = Window::GetSize().y;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 1;
    swapChainDesc.OutputWindow = Window::GetHandle();
    swapChainDesc.Windowed = true;

    HRESULT result;
    for (size_t i = 0; i < driverTypes.size(); i++)
    {
        result = D3D11CreateDeviceAndSwapChain(nullptr, driverTypes[i], nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &swapChainDesc, 
            swapChain.GetAddressOf(), device.GetAddressOf(), nullptr, context.GetAddressOf());

        if (SUCCEEDED(result))
            break;
    }

    if (FAILED(result))
        return;

    D3D11_VIEWPORT viewport = {};
    viewport.Width = (float)Window::GetSize().x;
    viewport.Height = (float)Window::GetSize().y;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    context->RSSetViewports(1, &viewport);
}

ID3D11Device& Graphics::GetDevice()
{
    return *device.Get();
}

ID3D11DeviceContext& Graphics::GetContext()
{
    return *context.Get();
}

IDXGISwapChain& Graphics::GetSwapChain()
{
    return *swapChain.Get();
}

IWICImagingFactory& Graphics::GetImageFactory()
{
    return *imageFactory.Get();
}

void Graphics::Update()
{
    swapChain->Present(data->VSync? 1 : 0, 0);
}
