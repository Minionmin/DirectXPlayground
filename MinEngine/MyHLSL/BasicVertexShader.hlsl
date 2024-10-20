#include "BasicShaderStructure.hlsli"

struct VS_INPUT
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
};

VS_OUTPUT BasicVS(VS_INPUT input)
{
    VS_OUTPUT output;
    const matrix wvpMat = mul(mul(projMat, viewMat), mul(transMat, rotMat)); // ワールドスペースに変換するための行列を計算します

    output.pos = mul(wvpMat, float4(input.pos, 1.0f)); // シェーダの行列演算は列優先であるため、左方向にかけていきます -> x,y,z,(w = 1)
    //output.pos = mul(float4(input.pos, 1.0f), wvpMat); // 上の方法を使いたくなければ、予め行列を転置しておくこともできます

    float4 tmpNormal = mul(rotMat, float4(input.normal, 1.0f)); // 法線ベクトルもワールドスペースに変換します
    //float4 tmpNormal = mul(float4(input.normal, 1.0f), wvpMat);
	output.normal = tmpNormal.xyz;

	return output;
}
