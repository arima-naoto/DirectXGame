#include "BasicShaderHeader.hlsli"

float4 BasicPS(Output input) : SV_TARGET
{
    float3 light = normalize(float3(1, -1, 1));
    float brightbess = dot(-light, input.normal.xyz);
    
    return float4(brightbess, brightbess, brightbess, 1);
}