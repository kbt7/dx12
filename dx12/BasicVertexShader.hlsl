#include "BasicType.hlsli"

cbuffer cbuff0 : register(b0)
{
    matrix mat; // �ϊ��s��
}

struct VSOutput
{
    float4 pos : SV_POSITION; // �ʒu�i�K�{�j
    float2 uv : TEXCOORD; // UV
};

VSOutput BasicVS(float4 pos : POSITION, float2 uv : TEXCOORD)
{
    VSOutput output;
    output.pos = mul(mat, pos); // �ϊ��s��ňʒu�ϊ�
    output.uv = uv;
    return output;
}
