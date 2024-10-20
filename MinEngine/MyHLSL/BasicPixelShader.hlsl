#include "BasicShaderStructure.hlsli"

float4 BasicPS(VS_OUTPUT input) : SV_TARGET
{
    // Lamburtian反射モデルを使って、ライトの方向と法線ベクトルからライトの強さを計算します
    float3 N = normalize(input.normal);

    // lightDirection: 光源からオブジェクトへの方向ベクトル
    // -lightDirection: オブジェクトから光源への方向ベクトル
    // lightDirectionとオブジェクトの法線ベクトルが同じ向きである場合、内積のcos値が1になり（cos0 = 1）、光源からの光が最も強くなります
    float3 L = normalize(-lightDirection);
    
    // 法線ベクトルとライトの方向ベクトルの内積を計算しする
	// 内積が - の場合、ライトが裏側から当たっていることを意味するので、0にします
    // 内積が 0 の場合、ライトと法線ベクトルが同じ向き（平行）であることを意味するので、0にします
    // 内積が + の場合、ライトが表側から当たっていることを意味するので、その値を使います
    float NdotL = max(dot(N, L), 0.0f);

    // オブジェクトの基本色
    float3 baseColor = float3(1.0f, 1.0f, 1.0f);

    // 光源の強度
    float lightIntensity = 1.5f;

    // アンビエントライト
    float3 ambient = ambientColor;
    
    // 光度の計算
    float3 diffuse = baseColor * lightColor * NdotL * lightIntensity;

    float3 finalColor = ambient + diffuse;
    
    return float4(finalColor, 1.0f);
}