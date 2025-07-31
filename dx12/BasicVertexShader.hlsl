#include "BasicType.hlsli"

cbuffer cbuff0 : register(b0)
{
    matrix mat; // 変換行列
}

struct VSOutput
{
    float4 pos : SV_POSITION; // 位置（必須）
    float2 uv : TEXCOORD; // UV
};

VSOutput BasicVS(float4 pos : POSITION, float2 uv : TEXCOORD)
{
    VSOutput output;
    output.pos = mul(mat, pos); // 変換行列で位置変換
    output.uv = uv;
    return output;
}
