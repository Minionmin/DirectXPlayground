cbuffer cbuff0 : register(b0)
{
    // wvpMatrices構造体と同じ順序
    float4x4 transMat;
    float4x4 rotMat;
    float4x4 scaleMat;
    float4x4 viewMat;
    float4x4 projMat;

    float3 lightDirection;
    float3 lightColor;
    float3 ambientColor;
};

// 頂点シェーダーからピクセルシェーダーへのやり取りに使用する構造体
struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
};