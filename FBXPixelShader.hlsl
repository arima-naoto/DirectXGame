Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

float4 main(PSInput input) : SV_TARGET
{
    float4 texColor = texture0.Sample(sampler0, input.uv);
    float brightness = saturate(dot(input.normal, float3(0, 1, 0)));
    float3 lighting = texColor.rgb * brightness;
    
    return float4(lighting, texColor.a);

}
