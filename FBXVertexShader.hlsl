struct VSInput
{
    float3 position : POSITION;
};

struct PSInput
{
    float4 position : SV_POSITION;
};

cbuffer TransformBuffer : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
};

PSInput main(VSInput input)
{
    PSInput output;
    matrix mvp = mul(projection, mul(view, world));
    output.position = mul(mvp, float4(input.position, 1.0f));
    return output;
}