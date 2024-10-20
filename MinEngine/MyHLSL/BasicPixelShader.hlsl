#include "BasicShaderStructure.hlsli"

float4 BasicPS(VS_OUTPUT input) : SV_TARGET
{
    // Lamburtian���˃��f�����g���āA���C�g�̕����Ɩ@���x�N�g�����烉�C�g�̋������v�Z���܂�
    float3 N = normalize(input.normal);

    // lightDirection: ��������I�u�W�F�N�g�ւ̕����x�N�g��
    // -lightDirection: �I�u�W�F�N�g��������ւ̕����x�N�g��
    // lightDirection�ƃI�u�W�F�N�g�̖@���x�N�g�������������ł���ꍇ�A���ς�cos�l��1�ɂȂ�icos0 = 1�j�A��������̌����ł������Ȃ�܂�
    float3 L = normalize(-lightDirection);
    
    // �@���x�N�g���ƃ��C�g�̕����x�N�g���̓��ς��v�Z������
	// ���ς� - �̏ꍇ�A���C�g���������瓖�����Ă��邱�Ƃ��Ӗ�����̂ŁA0�ɂ��܂�
    // ���ς� 0 �̏ꍇ�A���C�g�Ɩ@���x�N�g�������������i���s�j�ł��邱�Ƃ��Ӗ�����̂ŁA0�ɂ��܂�
    // ���ς� + �̏ꍇ�A���C�g���\�����瓖�����Ă��邱�Ƃ��Ӗ�����̂ŁA���̒l���g���܂�
    float NdotL = max(dot(N, L), 0.0f);

    // �I�u�W�F�N�g�̊�{�F
    float3 baseColor = float3(1.0f, 1.0f, 1.0f);

    // �����̋��x
    float lightIntensity = 1.5f;

    // �A���r�G���g���C�g
    float3 ambient = ambientColor;
    
    // ���x�̌v�Z
    float3 diffuse = baseColor * lightColor * NdotL * lightIntensity;

    float3 finalColor = ambient + diffuse;
    
    return float4(finalColor, 1.0f);
}