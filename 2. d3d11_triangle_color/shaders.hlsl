struct vs_in	// vertex shader in
{
	float3 position_local : POSITION;
	float3 color : COLOR;
};

struct vs_out	// vertex shader out
{
	float4 position_clip : SV_POSITION;
	float4 color : COLOR;
};

// entry point for vertex shader
vs_out vs_main(vs_in input)
{
	vs_out output = (vs_out)0;	// clear memory
	output.position_clip = float4(input.position_local, 1.0);
	output.color = float4(input.color, 1.0);
	return output;
}

// entry point for pixel shader
float4 ps_main(vs_out input) : SV_TARGET
{
	return input.color;	// returning RGBA color
}
