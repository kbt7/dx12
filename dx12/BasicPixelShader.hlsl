#include "BasicType.hlsli"

cbuffer CBAlpha : register(b0)
{
    float4x4 gTransform;
    float gAlpha;
    float gBrightness; // ★追加（明るさ係数）
    float padding[2]; // 16バイトアラインメント調整
};

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

float4 BasicPS(BasicType input) : SV_TARGET
{
    float4 color = g_texture.Sample(g_sampler, input.uv);
    color.rgb *= gBrightness; // ★ここで明るさを反映
    color.a *= gAlpha;
    return color;
}
