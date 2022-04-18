cbuffer GlobalData : register(b0)
{
	float4x4 gViewProj;
	float4 padding[12];
};

struct VSOutput
{
	float4 position : SV_Position;
	float4 color : TEXCOORD0;
};

struct VSInput
{
	float4 position : POSITION;
	float4 color    : COLOR;
};

VSOutput vs_main(VSInput input)
{
	VSOutput output;
	output.color = input.color;
	output.position = mul(gViewProj, float4(input.position.xyz, 1));
	return output;
}

float4 ps_main(VSOutput input) : SV_Target
{
	return input.color;
}