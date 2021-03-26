#include "library.h"

Camera::Camera()
{
    InitializeLibrary();


    ResizeBackBuffer();
}

void Camera::Start() const
{
    DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixTranspose(
        DirectX::XMMatrixInverse(
            nullptr,
            DirectX::XMMatrixRotationRollPitchYaw(
                DirectX::XMConvertToRadians(rotation.x),
                DirectX::XMConvertToRadians(rotation.y),
                DirectX::XMConvertToRadians(rotation.z)
            ) *
            DirectX::XMMatrixTranslation(position.x, position.y, position.z)
        )
    );

    cbuffer.Get().cameraWorldPosition = DirectX::XMFLOAT4(position.x, position.y, position.z, 1);

    DirectX::XMStoreFloat4x4(&cbuffer.Get().viewMatrix, viewMatrix);

    cbuffer.Attach(1);

    Graphics::GetContext().OMSetDepthStencilState(depthState.Get(), 0);
    Graphics::GetContext().ClearDepthStencilView(depthSteniclView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    Graphics::GetContext().OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthSteniclView.Get());

    float clearColor[4] = { color.x, color.y, color.z, color.w };
    Graphics::GetContext().ClearRenderTargetView(renderTargetView.Get(), clearColor);

}

void Camera::Stop() const
{
}

void Camera::ResizeBackBuffer()
{
    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture = nullptr;
    Graphics::GetSwapChain().GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(texture.GetAddressOf()));

    Graphics::GetDevice().CreateRenderTargetView(texture.Get(), nullptr, renderTargetView.GetAddressOf());

    // Depth resource
    D3D11_TEXTURE2D_DESC descDepth{};
    descDepth.Width = Window::GetSize().x;
    descDepth.Height = Window::GetSize().y;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_R32_TYPELESS;
    descDepth.SampleDesc.Count = 1;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    ThrowIfFailed(Graphics::GetDevice().CreateTexture2D(&descDepth, nullptr, &depthTex));

    // Depth DSV
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV{};
    descDSV.Format = DXGI_FORMAT_D32_FLOAT;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    ThrowIfFailed(Graphics::GetDevice().CreateDepthStencilView(depthTex.Get(), &descDSV, depthSteniclView.GetAddressOf()));

    D3D11_DEPTH_STENCIL_DESC desc{};
    desc.DepthEnable = true;
    desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    desc.StencilEnable = false;
    desc.FrontFace.StencilFailOp = desc.FrontFace.StencilDepthFailOp = desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    desc.BackFace = desc.FrontFace;
    Graphics::GetDevice().CreateDepthStencilState(&desc, &depthState);


    DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixTranspose(
        DirectX::XMMatrixPerspectiveFovLH(
            DirectX::XMConvertToRadians(60.0f),
            (float)Window::GetSize().x / (float)Window::GetSize().y,
            0.1f,
            1000.0f
        )
    );

    DirectX::XMStoreFloat4x4(&cbuffer.Get().projectionMatrix, projectionMatrix);

}
