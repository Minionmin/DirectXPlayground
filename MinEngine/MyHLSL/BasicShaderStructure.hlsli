cbuffer cbuff0 : register(b0)
{
    // wvpMatrices�\���̂Ɠ�������
    float4x4 transMat;
    float4x4 rotMat;
    float4x4 scaleMat;
    float4x4 viewMat;
    float4x4 projMat;

    float3 lightDirection;
    float3 lightColor;
    float3 ambientColor;
};

// ���_�V�F�[�_�[����s�N�Z���V�F�[�_�[�ւ̂����Ɏg�p����\����
struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
};