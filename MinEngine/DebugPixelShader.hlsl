#include "BasicShaderStructure.hlsli"

float4 DebugPS(VS_OUTPUT input) : SV_TARGET
{
	return float4(0.0f, 1.0f, 0.0f, 1.0f);
}
