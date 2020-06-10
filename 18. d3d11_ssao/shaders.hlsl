// Deferred Pass

cbuffer Movement : register(b0)
{
	matrix m_WorldViewProj;
	matrix m_World;
	matrix m_View;
	matrix m_Proj;
	matrix m_WorldView;
	float4 samples[64];
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

	// SSAO
	float4 positionVS : VSPOS;
	float4 normalVS : VSNOR;
};

vs_out vs_main(vs_in input)
{
	vs_out output = (vs_out)0;

	output.positionCS = mul(float4(input.positionLS, 1.0f), m_WorldViewProj);
	//output.positionCS = mul(mul(mul(float4(input.positionLS, 1.0f), m_World), m_View), m_Proj);
	output.positionN = input.positionN;
	output.color = input.color;
	output.positionWS = mul(float4(input.positionLS, 1.0f), m_World);

	// SSAO
	//output.positionVS = mul(mul(float4(input.positionLS, 1.0f), m_World), m_View);
	//output.normalVS = mul(mul(float4(input.positionN, 1.0f), m_World), m_View);
	output.positionVS = mul(float4(input.positionLS, 1.0f), m_WorldView);			// w = 1 point	// if w = 0 then I get the green, black, yellow, red color output like tutorials suggest??
	output.normalVS = mul(float4(input.positionN, 0.0f), m_WorldView);				// w = 0 direction

	return output;
}


struct DeferredPixelOutput
{
	float4 positionWS : SV_TARGET0;
	float4 positionN : SV_TARGET1;
	float4 color : SV_TARGET2;

	// SSAO
	float4 positionVS : SV_TARGET3;
	float4 normalVS: SV_TARGET4;
};

DeferredPixelOutput ps_main(vs_out input) : SV_TARGET
{
	DeferredPixelOutput output = (DeferredPixelOutput)0;
	output.positionWS = input.positionWS;
	output.positionN = float4(normalize(input.positionN), 1.f);
	output.color = float4(input.color, 1.f);
	output.positionVS = input.positionVS;
	output.normalVS = normalize(input.normalVS);

	return output;
}