/* World to Cubemap space:
 * Example:
 *   float3 env = environmentTexture.Sample(environmentSampler, mul(SwapZYMat, R)).rgb;
 */
static const float3x3 SwapZYMat = float3x3(float3(1, 0, 0), float3(0, 0, 1), float3(0, 1, 0));

float3 getWorldNormalFromTexture(Texture2D normalTexture, SamplerState normalSampler, float3 worldNormal, float3 worldPositon, float2 uv)
{
    float3 T, B;

    worldNormal = normalize(worldNormal);
    T = cross(ddy(worldPositon), worldNormal) * ddx(uv.x) + cross(worldNormal, ddx(worldPositon)) * ddy(uv.x);
    T = normalize(T - worldNormal * dot(T, worldNormal));
    B = normalize(cross(T, worldNormal));

    float3x3 TBN = float3x3(T, B, worldNormal);

    float3 normalTangentSpace = normalTexture.Sample(normalSampler, uv).rgb * 2 - float3(1, 1, 1);
    float3 normalWorldSpace = mul(normalTangentSpace, TBN);

    return normalWorldSpace;
}

float3 pack(float3 N)
{
    return N * 0.5 + float3(0.5, 0.5, 0.5);
}

float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
float3 srgbInv(float3 v)
{
    return float3(pow(abs(v.x), 2.2), pow(abs(v.y), 2.2), pow(abs(v.z), 2.2));
}
float3 srgb(float3 v)
{
    return float3(pow(abs(v.x), 0.45), pow(abs(v.y), 0.45), pow(abs(v.z), 0.45));
}