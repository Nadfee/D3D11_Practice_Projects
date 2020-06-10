cbuffer constants
{
	matrix first_mat;
	matrix second_mat;
};

struct vs_in	// vertex shader in
{
	float3 position_local : POSITION;
};

struct vs_out	// vertex shader out
{
	float4 position_clip : SV_POSITION;
};

// entry point for vertex shader
vs_out vs_main(vs_in input)
{
	vs_out output = (vs_out)0;	// clear memory

	//float4 before_clip_space = mul(float4(input.position_local, 1.0f), first_mat);
	//float4 in_clip_space = mul(before_clip_space, second_mat);
	//output.position_clip = in_clip_space;

	//output.position_clip = mul(float4(input.position_local, 1.0f), first_mat);
	float4 tmp = mul(float4(input.position_local, 1.0f), second_mat);
	output.position_clip = mul(tmp, first_mat);

	return output;
}

// entry point for pixel shader
float4 ps_main(vs_out input) : SV_TARGET
{
	return float4(1.0, 1.0, 1.0, 1.0);	// returning RGBA color
}
