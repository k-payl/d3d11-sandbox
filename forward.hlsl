#include "..\..\utils.hlsl"

cbuffer Object : register(b0)
{
    float4x4 modelMatrix; // Model space -> World space
    float4 color;
    float4 vars; // variabe 1, variable 2, variable 3 variable 4
    float4 scale;
};

cbuffer Camera : register(b1)
{
    float4x4 viewMatrix; // World space -> View space
    float4x4 projectionMatrix; // View space -> Clip space
    float4 cameraWorldPosition;
};

struct VertexIn
{
    float4 position : POSITION;
    float2 uv : TEXCOORD;
    float3 modelNormal : TEXCOORD1;
};

struct VertexOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 worldNormal : TEXCOORD1;
    float3 worldPositon : TEXCOORD2;
    float3 objectNormal : TEXCOORD3;
    float3 objectPosition : TEXCOORD4;
};

//--------------------------------------------
// Vertex shader
//--------------------------------------------
VertexOut VSMain(VertexIn vertex)
{
    VertexOut output;

    output.objectPosition = vertex.position.xyz;
    output.worldPositon = mul(float4(vertex.position.xyz * scale.x, 1), modelMatrix).xyz;
    output.position = mul(float4(output.worldPositon, 1), viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    output.uv = vertex.uv;
    output.objectNormal = vertex.modelNormal;
    output.worldNormal = mul(float4(vertex.modelNormal, 0), modelMatrix).xyz;

    return output;
}

/*
 * Textures
 * Example:
 *   float3 albedo = albedoTexture.Sample(mainSampler, attr.uv).rgb;
 */
Texture2D albedoTexture : register(t0);
SamplerState albedoSampler : register(s0);

Texture2D normalTexture : register(t1);
SamplerState normalSampler : register(s1);

Texture2D metalicTexture : register(t2);
SamplerState metalicSampler : register(s2);

TextureCube environmentTexture : register(t5);
SamplerState environmentSampler : register(s5);


//--------------------------------------------
// Pixel shader
//--------------------------------------------
float4 PSMain(VertexOut attr) : SV_TARGET
{
    return float4(attr.uv, 0, 1);
}
