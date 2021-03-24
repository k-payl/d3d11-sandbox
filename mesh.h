#pragma once
#include "library.h"


class Mesh
{
public:
    struct GPUVertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT2 uv;
        DirectX::XMFLOAT3 normal;

        GPUVertex() = default;

        GPUVertex(DirectX::XMFLOAT3 position, DirectX::XMFLOAT2 uv, DirectX::XMFLOAT3 n)
        {
            this->position = position;
            this->uv = uv;
            this->normal = n;
        }
    };

    DirectX::XMFLOAT3 position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    DirectX::XMFLOAT3 rotation = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    DirectX::XMFLOAT3 scale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
    Texture& texture = Texture::GetEmpty();
    Texture* normalTexture{};
    Texture* metallicTexture{};
    Texture *cubeTexture;
    Shader* shader;
#pragma pack(push, 1)
    struct MeshConstantBuffer
    {
        DirectX::XMFLOAT4X4 modelMatrix;
        DirectX::XMFLOAT4 albedo = {1,1,1,1};
        DirectX::XMFLOAT4 vars = { 1,1,1,1 };
        DirectX::XMFLOAT4 scale = { 1,1,1,1 };
    };
#pragma pack(pop)
    CBuffer<MeshConstantBuffer> cbuffer;

    Mesh(const std::vector<GPUVertex>& vertices, const std::vector<UINT>& indices);
    void Draw() const;

    static std::unique_ptr<Mesh> CreatePlane(DirectX::XMFLOAT2 size);
    static std::unique_ptr<Mesh> CreateCube(DirectX::XMFLOAT3 size);
    static std::unique_ptr<Mesh> CreateFromFile(std::wstring path);

private:

    UINT vertexCount;
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
};
