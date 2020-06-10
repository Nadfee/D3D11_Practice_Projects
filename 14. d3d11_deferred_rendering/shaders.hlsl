// Deferred Pass

cbuffer Movement : register(b0)
{
	matrix m_WorldViewProj;
	matrix m_World;
};

cbuffer Light : register(b1)
{
	float3 lightPos;
	float padding;
	float3 cameraPos;
	float padding2;
};

struct vs_in
{
	float3 positionLS : POSITION;
	float3 positionN : NORMAL;
	float3 color : COLOR;
};

struct vs_out
{
	float4 positionCS : SV_POSITION;
	float3 positionN : S_NORMAL;
	float3 color : COLOR;
	float4 positionWS : RPOS;
};

vs_out vs_main(vs_in input)
{
	vs_out output = (vs_out)0;

	output.positionCS = mul(float4(input.positionLS, 1.0f), m_WorldViewProj);
	output.positionN = input.positionN;
	output.color = input.color;
	output.positionWS = mul(float4(input.positionLS, 1.0f), m_World);

	return output;
}


struct DeferredPixelOutput
{
	float4 positionWS : SV_TARGET0;
	float4 positionN : SV_TARGET1;
	float4 color : SV_TARGET2;
};

DeferredPixelOutput ps_main(vs_out input) : SV_TARGET
{
	DeferredPixelOutput output = (DeferredPixelOutput)0;
	output.positionWS = input.positionWS;
	output.positionN = float4(input.positionN, 1.f);
	output.color = float4(input.color, 1.f);

	return output;
}