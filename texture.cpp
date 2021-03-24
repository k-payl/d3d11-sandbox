#include "library.h"
#include <filesystem>

namespace fs = std::filesystem;
#undef max

int mipmapsNumber(int width, int height) // rounding down rule
{
    return 1 + (int)floor(log2((float)std::max(width, height)));
}

UINT f2Bytes(DXGI_FORMAT format)
{
    switch (format)
    {
    case DXGI_FORMAT_B8G8R8A8_UNORM: return 4;
    case DXGI_FORMAT_R16G16B16A16_FLOAT: return 2 * 4;
    default: abort();
    }
}

Texture::Texture(const void* buffer, int width, int height, bool cube, DXGI_FORMAT format)
{
    assert(format == DXGI_FORMAT_B8G8R8A8_UNORM || format == DXGI_FORMAT_R16G16B16A16_FLOAT);

    InitializeLibrary();

    size = DirectX::XMUINT2(width, height);

    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = size.x;
    textureDesc.Height = size.y;
    textureDesc.MipLevels = mipmapsNumber(size.x, size.y);
    textureDesc.ArraySize = (cube ? 6 : 1);
    textureDesc.Format = format;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | 
        D3D11_BIND_RENDER_TARGET; // need for GenerateMips
    textureDesc.MiscFlags = (cube? D3D11_RESOURCE_MISC_TEXTURECUBE : 0) | D3D11_RESOURCE_MISC_GENERATE_MIPS;

    //D3D11_SUBRESOURCE_DATA textureSubresourceData = {};
    //textureSubresourceData.pSysMem = buffer;
    //textureSubresourceData.SysMemPitch = size.x * 4;
    //textureSubresourceData.SysMemSlicePitch = size.x * size.y * 4;
    // Initial data is NULL
    ThrowIfFailed(Graphics::GetDevice().CreateTexture2D(&textureDesc, NULL, texture.GetAddressOf()));

    // Upload data for 0-th mip
    const uint8_t* pSrc = (uint8_t*)buffer;
    for (UINT arraySlice = 0; arraySlice < (cube? 6u : 1u); ++arraySlice)
    {
        size_t mipBytes = size.x * size.y * f2Bytes(format);

        UINT res = D3D11CalcSubresource(0, arraySlice, mipmapsNumber(size.x, size.y));

        Graphics::GetContext().UpdateSubresource(texture.Get(), res, nullptr, 
            pSrc, static_cast<UINT>(size.x * f2Bytes(format)), static_cast<UINT>(mipBytes));

        pSrc += mipBytes;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
    shaderResourceViewDesc.Format = format;
    shaderResourceViewDesc.ViewDimension = cube? D3D11_SRV_DIMENSION_TEXTURECUBE : D3D11_SRV_DIMENSION_TEXTURE2D;

    if (cube)
    {
        shaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
        shaderResourceViewDesc.TextureCube.MipLevels = mipmapsNumber(size.x, size.y);
    }
    else
    {
        shaderResourceViewDesc.Texture2D.MipLevels = mipmapsNumber(size.x, size.y);
    }
    ThrowIfFailed(Graphics::GetDevice().CreateShaderResourceView(texture.Get(), &shaderResourceViewDesc, shaderResourceView.GetAddressOf()));

    Graphics::GetContext().GenerateMips(shaderResourceView.Get());

    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.MinLOD = 0.0f;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    ThrowIfFailed(Graphics::GetDevice().CreateSamplerState(&samplerDesc, samplerState.GetAddressOf()));
}

ID3D11Texture2D& Texture::GetTexture() const
{
    return *texture.Get();
}

DirectX::XMUINT2 Texture::GetSize() const
{
    return size;
}

void Texture::Attach(int slot) const
{
    Graphics::GetContext().PSSetShaderResources(slot, 1, shaderResourceView.GetAddressOf());
    Graphics::GetContext().PSSetSamplers(slot, 1, samplerState.GetAddressOf());
}

Texture& Texture::GetEmpty()
{
    static std::unique_ptr<Texture> texture = nullptr;

    if (texture == nullptr)
    {
        std::uint32_t buffer = 0x00000000;
        texture = std::make_unique<Texture>(&buffer, 1, 1, false, DXGI_FORMAT_B8G8R8A8_UNORM);
    }

    return *texture.get();
}

std::unique_ptr<Texture> Texture::Create2D(const std::wstring& filePath)
{
    InitializeLibrary();

    Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder = nullptr;
    Graphics::GetImageFactory().CreateDecoderFromFilename(filePath.c_str(), 0, GENERIC_READ, WICDecodeMetadataCacheOnDemand, decoder.GetAddressOf());

    Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame = nullptr;
    decoder->GetFrame(0, &frame);

    UINT width, height;
    frame->GetSize(&width, &height);

    WICPixelFormatGUID pixelFormat;
    frame->GetPixelFormat(&pixelFormat);
    std::unique_ptr<BYTE[]> buffer(new BYTE[width * height * 4]);

    if (pixelFormat != GUID_WICPixelFormat32bppBGRA)
    {
        Microsoft::WRL::ComPtr<IWICFormatConverter> formatConverter = nullptr;
        Graphics::GetImageFactory().CreateFormatConverter(&formatConverter);

        formatConverter->Initialize(frame.Get(), GUID_WICPixelFormat32bppBGRA, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom);

        formatConverter->CopyPixels(0, width * 4, width * height * 4, buffer.get());
    }
    else
    {
        frame->CopyPixels(0, width * 4, width * height * 4, buffer.get());
    }

    return std::make_unique<Texture>(buffer.get(), width, height, false, DXGI_FORMAT_B8G8R8A8_UNORM);
}

std::unique_ptr<Texture> Texture::CreateCube(const std::wstring& filePath)
{
#pragma pack(push,1)

    const uint32_t DDS_MAGIC = 0x20534444; // "DDS "

    struct DDS_PIXELFORMAT
    {
        uint32_t size;
        uint32_t flags;
        uint32_t fourCC;
        uint32_t RGBBitCount;
        uint32_t RBitMask;
        uint32_t GBitMask;
        uint32_t BBitMask;
        uint32_t ABitMask;
    };

#define DDS_FOURCC      0x00000004  // DDPF_FOURCC
#define DDS_RGB         0x00000040  // DDPF_RGB
#define DDS_LUMINANCE   0x00020000  // DDPF_LUMINANCE
#define DDS_ALPHA       0x00000002  // DDPF_ALPHA
#define DDS_BUMPDUDV    0x00080000  // DDPF_BUMPDUDV

#define DDS_HEADER_FLAGS_VOLUME 0x00800000  // DDSD_DEPTH

#define DDS_HEIGHT 0x00000002 // DDSD_HEIGHT
#define DDS_WIDTH  0x00000004 // DDSD_WIDTH

#define DDS_CUBEMAP_POSITIVEX 0x00000600 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEX
#define DDS_CUBEMAP_NEGATIVEX 0x00000a00 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEX
#define DDS_CUBEMAP_POSITIVEY 0x00001200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEY
#define DDS_CUBEMAP_NEGATIVEY 0x00002200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEY
#define DDS_CUBEMAP_POSITIVEZ 0x00004200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEZ
#define DDS_CUBEMAP_NEGATIVEZ 0x00008200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEZ

#define DDS_CUBEMAP_ALLFACES ( DDS_CUBEMAP_POSITIVEX | DDS_CUBEMAP_NEGATIVEX |\
                               DDS_CUBEMAP_POSITIVEY | DDS_CUBEMAP_NEGATIVEY |\
                               DDS_CUBEMAP_POSITIVEZ | DDS_CUBEMAP_NEGATIVEZ )

#define DDS_CUBEMAP 0x00000200 // DDSCAPS2_CUBEMAP

    enum DDS_MISC_FLAGS2
    {
        DDS_MISC_FLAGS2_ALPHA_MODE_MASK = 0x7L,
    };

    struct DDS_HEADER
    {
        uint32_t        size;
        uint32_t        flags;
        uint32_t        height;
        uint32_t        width;
        uint32_t        pitchOrLinearSize;
        uint32_t        depth; // only if DDS_HEADER_FLAGS_VOLUME is set in flags
        uint32_t        mipMapCount;
        uint32_t        reserved1[11];
        DDS_PIXELFORMAT ddspf;
        uint32_t        caps;
        uint32_t        caps2;
        uint32_t        caps3;
        uint32_t        caps4;
        uint32_t        reserved2;
    };

    struct DDS_HEADER_DXT10
    {
        DXGI_FORMAT     dxgiFormat;
        uint32_t        resourceDimension;
        uint32_t        miscFlag; // see D3D11_RESOURCE_MISC_FLAG
        uint32_t        arraySize;
        uint32_t        miscFlags2;
    };

#pragma pack(pop)
    std::ifstream tmp(filePath, std::ifstream::in | std::ifstream::binary);

    fs::path path(filePath);
    std::size_t size = fs::file_size(path);
    std::unique_ptr<BYTE[]> buffer(new BYTE[size]);

    tmp.read((char*)buffer.get(), size);

    const DDS_HEADER* header = reinterpret_cast<const DDS_HEADER*>(buffer.get() + sizeof(uint32_t));

    const bool mipmapsPresented = header->mipMapCount > 1;
    const UINT width = header->width;
    const UINT height = header->height;

    // Check for DX10 extension
    bool bDXT10Header = (header->ddspf.flags & DDS_FOURCC) && (MAKEFOURCC('D', 'X', '1', '0') == header->ddspf.fourCC);

    ptrdiff_t headerOffset = sizeof(uint32_t) + sizeof(DDS_HEADER) + (bDXT10Header ? sizeof(DDS_HEADER_DXT10) : 0);

    uint8_t* imageData = buffer.get() + headerOffset;
    size_t sizeInBytes = size - headerOffset;

    const DDS_HEADER_DXT10* d3d10ext = reinterpret_cast<const DDS_HEADER_DXT10*>((const char*)header + sizeof(DDS_HEADER));

    DXGI_FORMAT format{};

    if (bDXT10Header)
    {
        //assert(d3d10ext->arraySize == 6);
        assert(d3d10ext->resourceDimension == D3D11_RESOURCE_DIMENSION_TEXTURE2D);
        assert(d3d10ext->miscFlag & D3D11_RESOURCE_MISC_TEXTURECUBE);
        assert(d3d10ext->dxgiFormat == DXGI_FORMAT_R16G16B16A16_FLOAT);
		format = d3d10ext->dxgiFormat;
    }
    else
    {
		const DDS_PIXELFORMAT ddpf = header->ddspf;

		#ifndef MAKEFOURCC
		#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
						((uint32_t)(uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8) |       \
						((uint32_t)(uint8_t)(ch2) << 16) | ((uint32_t)(uint8_t)(ch3) << 24 ))
		#endif

		#define ISBITMASK( r,g,b,a ) ( ddpf.RBitMask == r && ddpf.GBitMask == g && ddpf.BBitMask == b && ddpf.ABitMask == a )

        assert(ddpf.flags& DDS_FOURCC);
        assert(ddpf.fourCC == 113);
        assert(!(header->flags& DDS_HEADER_FLAGS_VOLUME));
        assert(header->caps2& DDS_CUBEMAP);

        int y = 0;

    }

    return std::make_unique<Texture>(imageData, width, height, true, DXGI_FORMAT_R16G16B16A16_FLOAT);
}
