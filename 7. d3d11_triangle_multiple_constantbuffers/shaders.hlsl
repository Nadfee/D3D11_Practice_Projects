// Register keyword is important? For example, b0 takes in the FIRST cbuffer and b1 takes the SECOND cbuffer (try changing b0 and b1 and it changes)

cbuffer constants : register(b0)		// THESE REGISTERS ARE VERY IMPORTANT??
{
	matrix my_mat;
	matrix another_mat;
};

cbuffer other_consts : register(b1)		// THESE REGISTERS ARE VERY IMPORtANT??
{
	matrix new_my_mat;
	matrix new_another_mat;
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

	output.position_clip = mul(my_mat, float4(input.position_local, 1.0f));		
	//output.position_clip = mul(new_my_mat, float4(input.position_local, 1.0f));

	return output;
}

// entry point for pixel shader
float4 ps_main(vs_out input) : SV_TARGET
{
	return float4(1.0, 1.0, 1.0, 1.0);	// returning RGBA color
}
