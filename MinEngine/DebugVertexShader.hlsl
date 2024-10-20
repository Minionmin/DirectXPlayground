#include "BasicShaderStructure.hlsli"

struct VS_INPUT
{
	float3 pos : POSITION;
};

VS_OUTPUT DebugVS(VS_INPUT input)
{
    VS_OUTPUT output;
    const matrix wvpMat = mul(mul(projMat, viewMat), mul(transMat, rotMat));
    output.pos = mul(wvpMat, float4(input.pos, 1.0f));
	output.normal = float3(0.0f, 0.0f, 0.0f); // �@���x�N�g���͎g��Ȃ��̂�0�x�N�g������
    return output;
}