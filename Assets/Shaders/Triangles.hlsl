cbuffer myScene : register(b0)
{
	float4x4 mViewProj;
	float4   mEye;
	float4   padding[11];
};

cbuffer myObject : register(b1)
{
	float4x4 mWorld;
	float4x4 mWorldInversTrans;
	float4   mColor;
	float4 padding2[7];
};

struct VSOutput
{
	float4 position  : SV_Position;
	float3 positionW : TEXCOORD0;
	float3 normalW   : TEXCOORD1;
	float2 uv : TEXCOORD2;
};

struct VSInput
{
	float4 position : POSITION;
	float4 normal   : NORMAL;
	float4 texcoord : TEXCOORD;
};

VSOutput vs_main(VSInput input)
{
	VSOutput output;
#if VTX_PASSTHROUGH
	output.positionW = input.position.xyz;
	output.normalW = input.normal.xyz;
#else
	output.positionW = mul(mWorld, input.position).xyz;
	output.normalW   = normalize(mul(mWorldInversTrans, input.normal)).xyz;
#endif
	output.position = mul(mViewProj, float4(output.positionW, 1));

	output.uv = input.texcoord.xy;
	return output;
}

float4 ps_main(VSOutput input) : SV_Target
{
#if RENDER_NORMALS
	return float4(input.normalW * 0.5f + 0.5f, 1);
#elif RENDER_UVS
	return float4(input.uv, 0, 1);
#else
	float3 radiance = mColor.rgb * max(0, dot(input.normalW, normalize(mEye.xyz - input.positionW.xyz)));
	return float4(radiance, 1);
#endif
}