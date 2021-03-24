#pragma once
#include "library.h"

class Texture
{
public:
    Texture(const void* buffer, int width, int height, bool cube, DXGI_FORMAT f);
    ID3D11Texture2D& GetTexture() const;
    DirectX::XMUINT2 GetSize() const;
    void Attach(int slot) const;

    static Texture& GetEmpty();
    static std::unique_ptr<Texture> Create2D(const std::wstring& filePath);
    static std::unique_ptr<Texture> CreateCube(const std::wstring& filePath);

private:
    DirectX::XMUINT2 size;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture = nullptr;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView = nullptr;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState = nullptr;
};
