#include "library.h"

extern Data* data;

Shader::Shader(const std::string& source)
{
    InitializeLibrary();

    Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob = nullptr;
    Compile(source, "VSMain", "vs_5_0", vertexShaderBlob.GetAddressOf());

    Graphics::GetDevice().CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), nullptr, vertexShader.GetAddressOf());

    Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob = nullptr;
    Compile(source, "PSMain", "ps_5_0", pixelShaderBlob.GetAddressOf());

    Graphics::GetDevice().CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), nullptr, pixelShader.GetAddressOf());

    std::vector<D3D11_INPUT_ELEMENT_DESC> inputElementDesc
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    Graphics::GetDevice().CreateInputLayout(inputElementDesc.data(), (UINT)inputElementDesc.size(), vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), inputLayout.GetAddressOf());
}

void Shader::Attach() const
{
    Graphics::GetContext().VSSetShader(vertexShader.Get(), nullptr, 0);
    Graphics::GetContext().PSSetShader(pixelShader.Get(), nullptr, 0);
    Graphics::GetContext().IASetInputLayout(inputLayout.Get());
}

void Shader::Compile(const std::string& source, const char* entryPoint, const char* shaderModel, ID3DBlob** out)
{
    Microsoft::WRL::ComPtr<ID3DBlob> error = nullptr;
    D3DCompile(source.c_str(), source.length(), nullptr, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, shaderModel, D3DCOMPILE_ENABLE_STRICTNESS, 0, out, error.GetAddressOf());

    if (error != nullptr)
        throw std::string((char*)error->GetBufferPointer());
}

void CompileShaders()
{
    try
    {
        std::ifstream tmp("../../forward.hlsl", std::ifstream::in);

        std::string str((std::istreambuf_iterator<char>(tmp)),
            std::istreambuf_iterator<char>());

        data->forward = std::make_unique<Shader>(str);

        data->CompileError.clear();
    }
    catch (const std::string error)
    {
        data->CompileError = error;

        for (int i = 0; i < error.size(); i += 25)
        {
            data->CompileError.insert(data->CompileError.begin() + i, '\n');
        }

        data->forward = nullptr;
    }
}

