#include "library.h"
#include <filesystem>

namespace fs = std::filesystem;

Mesh::Mesh(const std::vector<GPUVertex>& vertices, const std::vector<UINT>& indices)
{
    InitializeLibrary();

    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.ByteWidth = sizeof(GPUVertex) * (UINT)vertices.size();
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexSubresourceData = {};
    vertexSubresourceData.pSysMem = vertices.data();

    Graphics::GetDevice().CreateBuffer(&vertexBufferDesc, &vertexSubresourceData, vertexBuffer.GetAddressOf());

    if (indices.empty())
    {
        vertexCount = (UINT)vertices.size();
    }
    else
    {
        vertexCount = (UINT)indices.size();

        D3D11_BUFFER_DESC indexBufferDesc = {};
        indexBufferDesc.ByteWidth = sizeof(UINT) * (UINT)indices.size();
        indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA indexSubresourceData = {};
        indexSubresourceData.pSysMem = indices.data();

        Graphics::GetDevice().CreateBuffer(&indexBufferDesc, &indexSubresourceData, indexBuffer.GetAddressOf());
    }
}

void Mesh::Draw() const
{
    DirectX::XMMATRIX modelMatrix = DirectX::XMMatrixTranspose(
        DirectX::XMMatrixScaling(scale.x, scale.y, scale.z) *
        DirectX::XMMatrixRotationRollPitchYaw(
            DirectX::XMConvertToRadians(rotation.x),
            DirectX::XMConvertToRadians(rotation.y),
            DirectX::XMConvertToRadians(rotation.z)
        ) *
        DirectX::XMMatrixTranslation(position.x, position.y, position.z)
    );

    DirectX::XMStoreFloat4x4(&cbuffer.Get().modelMatrix, modelMatrix);

    cbuffer.Attach(0);
    texture.Attach(0);
    normalTexture->Attach(1);
    metallicTexture->Attach(2);
    cubeTexture->Attach(5);

    if (!shader) // Don't draw if shader is not set or it has errors
        return;

    shader->Attach();

    UINT stride = sizeof(GPUVertex);
    UINT offset = 0;
    Graphics::GetContext().IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);

    if (indexBuffer)
        Graphics::GetContext().IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    else
        Graphics::GetContext().IASetIndexBuffer(NULL, DXGI_FORMAT_R32_UINT, 0);

    Graphics::GetContext().IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    if (indexBuffer)
        Graphics::GetContext().DrawIndexed(vertexCount, 0, 0);
    else
        Graphics::GetContext().Draw(vertexCount, 0);
}

std::unique_ptr<Mesh> Mesh::CreatePlane(DirectX::XMFLOAT2 size)
{
    size.x *= 0.5f;
    size.y *= 0.5f;

    std::vector<GPUVertex> vertices
    {
        GPUVertex(DirectX::XMFLOAT3(-size.x, 0.0f, size.y),   DirectX::XMFLOAT2(0.0f, 0.0f), DirectX::XMFLOAT3(0,1,0)),
        GPUVertex(DirectX::XMFLOAT3(size.x,  0.0f, size.y),   DirectX::XMFLOAT2(1.0f, 0.0f), DirectX::XMFLOAT3(0,1,0)),
        GPUVertex(DirectX::XMFLOAT3(-size.x, 0.0f, - size.y), DirectX::XMFLOAT2(0.0f, 1.0f), DirectX::XMFLOAT3(0,1,0)),
        GPUVertex(DirectX::XMFLOAT3(size.x,  0.0f, - size.y), DirectX::XMFLOAT2(1.0f, 1.0f), DirectX::XMFLOAT3(0,1,0))
    };

    std::vector<UINT> indices
    {
        0, 1, 2,
        3, 2, 1
    };

    return std::make_unique<Mesh>(vertices, indices);
}

std::unique_ptr<Mesh> Mesh::CreateCube(DirectX::XMFLOAT3 size)
{
    size.x *= 0.5f;
    size.y *= 0.5f;
    size.z *= 0.5f;

    std::vector<GPUVertex> vertices
    {
        // 前
        GPUVertex(DirectX::XMFLOAT3(-size.x, size.y,  -size.z), DirectX::XMFLOAT2(0.0f, 0.0f), DirectX::XMFLOAT3(0,0,-1)),
        GPUVertex(DirectX::XMFLOAT3(size.x, size.y,   -size.z), DirectX::XMFLOAT2(1.0f, 0.0f), DirectX::XMFLOAT3(0,0,-1)),
        GPUVertex(DirectX::XMFLOAT3(-size.x, -size.y, -size.z), DirectX::XMFLOAT2(0.0f, 1.0f), DirectX::XMFLOAT3(0,0,-1)),
        GPUVertex(DirectX::XMFLOAT3(size.x, -size.y,  -size.z), DirectX::XMFLOAT2(1.0f, 1.0f), DirectX::XMFLOAT3(0,0,-1)),

        // 後
        GPUVertex(DirectX::XMFLOAT3(size.x, size.y,   size.z), DirectX::XMFLOAT2(0.0f, 0.0f), DirectX::XMFLOAT3(0,0,1)),
        GPUVertex(DirectX::XMFLOAT3(-size.x, size.y,  size.z), DirectX::XMFLOAT2(1.0f, 0.0f), DirectX::XMFLOAT3(0,0,1)),
        GPUVertex(DirectX::XMFLOAT3(size.x, -size.y,  size.z), DirectX::XMFLOAT2(0.0f, 1.0f), DirectX::XMFLOAT3(0,0,1)),
        GPUVertex(DirectX::XMFLOAT3(-size.x, -size.y, size.z), DirectX::XMFLOAT2(1.0f, 1.0f), DirectX::XMFLOAT3(0,0,1)),

        // 左
        GPUVertex(DirectX::XMFLOAT3(size.x, size.y,  -size.z), DirectX::XMFLOAT2(0.0f, 0.0f), DirectX::XMFLOAT3(1,0,0)),
        GPUVertex(DirectX::XMFLOAT3(size.x, size.y,   size.z), DirectX::XMFLOAT2(1.0f, 0.0f), DirectX::XMFLOAT3(1,0,0)),
        GPUVertex(DirectX::XMFLOAT3(size.x, -size.y, -size.z), DirectX::XMFLOAT2(0.0f, 1.0f), DirectX::XMFLOAT3(1,0,0)),
        GPUVertex(DirectX::XMFLOAT3(size.x, -size.y,  size.z), DirectX::XMFLOAT2(1.0f, 1.0f), DirectX::XMFLOAT3(1,0,0)),

        // 右
        GPUVertex(DirectX::XMFLOAT3(-size.x, size.y,   size.z), DirectX::XMFLOAT2(0.0f, 0.0f), DirectX::XMFLOAT3(-1,0,0)),
        GPUVertex(DirectX::XMFLOAT3(-size.x, size.y,  -size.z), DirectX::XMFLOAT2(1.0f, 0.0f), DirectX::XMFLOAT3(-1,0,0)),
        GPUVertex(DirectX::XMFLOAT3(-size.x, -size.y,  size.z), DirectX::XMFLOAT2(0.0f, 1.0f), DirectX::XMFLOAT3(-1,0,0)),
        GPUVertex(DirectX::XMFLOAT3(-size.x, -size.y, -size.z), DirectX::XMFLOAT2(1.0f, 1.0f), DirectX::XMFLOAT3(-1,0,0)),

        // 上
        GPUVertex(DirectX::XMFLOAT3(-size.x, size.y,  size.z), DirectX::XMFLOAT2(0.0f, 0.0f), DirectX::XMFLOAT3(0,1,0)),
        GPUVertex(DirectX::XMFLOAT3(size.x, size.y,   size.z), DirectX::XMFLOAT2(1.0f, 0.0f), DirectX::XMFLOAT3(0,1,0)),
        GPUVertex(DirectX::XMFLOAT3(-size.x, size.y, -size.z), DirectX::XMFLOAT2(0.0f, 1.0f), DirectX::XMFLOAT3(0,1,0)),
        GPUVertex(DirectX::XMFLOAT3(size.x, size.y,  -size.z), DirectX::XMFLOAT2(1.0f, 1.0f), DirectX::XMFLOAT3(0,1,0)),

        // 下
        GPUVertex(DirectX::XMFLOAT3(-size.x, -size.y, -size.z), DirectX::XMFLOAT2(0.0f, 0.0f), DirectX::XMFLOAT3(0,-1,0)),
        GPUVertex(DirectX::XMFLOAT3(size.x, -size.y,  -size.z), DirectX::XMFLOAT2(1.0f, 0.0f), DirectX::XMFLOAT3(0,-1,0)),
        GPUVertex(DirectX::XMFLOAT3(-size.x, -size.y,  size.z), DirectX::XMFLOAT2(0.0f, 1.0f), DirectX::XMFLOAT3(0,-1,0)),
        GPUVertex(DirectX::XMFLOAT3(size.x, -size.y,   size.z), DirectX::XMFLOAT2(1.0f, 1.0f), DirectX::XMFLOAT3(0,-1,0))
    };

    std::vector<UINT> indices
    {
        // 前
        0, 1, 2,
        3, 2, 1,

        // 後
        4, 5, 6,
        7, 6, 5,

        // 左
        8, 9, 10,
        11, 10, 9,

        // 右
        12, 13, 14,
        15, 14, 13,

        // 上
        16, 17, 18,
        19, 18, 17,

        // 下
        20, 21, 22,
        23, 22, 21
    };

    return std::make_unique<Mesh>(vertices, indices);
}

std::unique_ptr<Mesh> Mesh::CreateFromFile(std::wstring filePath)
{
    std::ifstream tmp(filePath, std::ifstream::in | std::ifstream::binary);

    fs::path path(filePath);
    std::size_t size = fs::file_size(path);
    std::vector<GPUVertex> vertices(size / sizeof(GPUVertex));

    tmp.read((char*)vertices.data(), size);

    int y = 0;
    const std::vector<UINT> indices;

    return std::make_unique<Mesh>(vertices, indices);
}

