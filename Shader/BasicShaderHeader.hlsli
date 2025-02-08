Texture2D<float4> tex : register(t0);
SamplerState smp : register(s0);

cbuffer cbff : register(b0)
{
    matrix mat;
};

struct Output
{
    float4 svpos : SV_POSITION;
    float2 uv : TEXCOORD;
};