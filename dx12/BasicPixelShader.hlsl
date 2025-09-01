#include "BasicType.hlsli"

cbuffer CBAlpha : register(b0)
{
    float4x4 gTransform; // �ϊ��s��
    float gAlpha; // ���ߓx
    float padding[3]; // 16�o�C�g�A���C�������g
};

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

float4 BasicPS(BasicType input) : SV_TARGET
{
    float4 color = g_texture.Sample(g_sampler, input.uv);
    color.a *= gAlpha; // alpha �𔽉f
    return color;
}
