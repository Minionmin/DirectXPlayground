#include "BasicShaderStructure.hlsli"

struct VS_INPUT
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
};

VS_OUTPUT BasicVS(VS_INPUT input)
{
    VS_OUTPUT output;
    const matrix wvpMat = mul(mul(projMat, viewMat), mul(transMat, rotMat)); // ���[���h�X�y�[�X�ɕϊ����邽�߂̍s����v�Z���܂�

    output.pos = mul(wvpMat, float4(input.pos, 1.0f)); // �V�F�[�_�̍s�񉉎Z�͗�D��ł��邽�߁A�������ɂ����Ă����܂� -> x,y,z,(w = 1)
    //output.pos = mul(float4(input.pos, 1.0f), wvpMat); // ��̕��@���g�������Ȃ���΁A�\�ߍs���]�u���Ă������Ƃ��ł��܂�

    float4 tmpNormal = mul(rotMat, float4(input.normal, 1.0f)); // �@���x�N�g�������[���h�X�y�[�X�ɕϊ����܂�
    //float4 tmpNormal = mul(float4(input.normal, 1.0f), wvpMat);
	output.normal = tmpNormal.xyz;

	return output;
}
