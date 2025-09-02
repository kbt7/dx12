#include "BasicType.hlsli"

cbuffer CBAlpha : register(b0)
{
    float4x4 gTransform;
    float gAlpha;
    float gBrightness; // ���ǉ��i���邳�W���j
    float padding[2]; // 16�o�C�g�A���C�������g����
};

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

float4 BasicPS(BasicType input) : SV_TARGET
{
    float4 color = g_texture.Sample(g_sampler, input.uv);
    color.rgb *= gBrightness; // �������Ŗ��邳�𔽉f
    color.a *= gAlpha;
    return color;
}
