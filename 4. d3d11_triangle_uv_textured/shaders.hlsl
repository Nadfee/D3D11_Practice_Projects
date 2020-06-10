struct vs_in	// vertex shader in
{
	float3 position_local : POSITION;
	float2 uv : TEX;
};

struct vs_out	// vertex shader out
{
	float4 position_clip : SV_POSITION;
	float2 uv : TEX;
};

Texture2D my_texture2d;
SamplerState my_sampler_state;

// entry point for vertex shader
vs_out vs_main(vs_in input)
{
	vs_out output = (vs_out)0;	// clear memory
	output.position_clip = float4(input.position_local, 1.0);
	output.uv = input.uv;
	return output;
}

// entry point for pixel shader
float4 ps_main(vs_out input) : SV_TARGET
{
	//return float4(input.uv, 0.0f, 1.0);	// returning RGBA color
	return my_texture2d.Sample(my_sampler_state, input.uv);
}
