#include "BasicType.hlsli"

cbuffer CBAlpha : register(b0)
{
    float4x4 gTransform; // 変換行列
    float gAlpha; // 透過度
    float padding[3]; // 16バイトアラインメント
};

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

float4 BasicPS(BasicType input) : SV_TARGET
{
    float4 color = g_texture.Sample(g_sampler, input.uv);
    color.a *= gAlpha; // alpha を反映
    return color;
}
