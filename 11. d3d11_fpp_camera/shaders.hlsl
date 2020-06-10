cbuffer Movement : register(b0)
{
	matrix m_WorldViewProj;
};


cbuffer Light : register(b1)
{
	float3 light_pos;
	float padding;
};

struct vs_in
{
	float3 position_local : POSITION;
	float3 position_normal : NORMAL;
	float2 uv : TEX;
};

struct vs_out
{
	float4 position_clip : SV_POSITION;
	float3 position_normal : S_NORMAL;
	float2 uv : TEX;
};

Texture2D my_texture2d;
SamplerState my_sampler_state;

vs_out vs_main(vs_in input)
{
	vs_out output = (vs_out)0;

	output.position_clip = mul(float4(input.position_local, 1.0f), m_WorldViewProj);
	output.position_normal = input.position_normal;
	output.uv = input.uv;

	return output;
}

float4 ps_main(vs_out input) : SV_TARGET
{

	return float4(1.f, 1.f, 1.f, 1.f);
}
