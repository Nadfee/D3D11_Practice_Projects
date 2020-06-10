cbuffer Movement : register(b0)
{
	matrix mov_mat;
};


cbuffer Light : register(b1)
{
	float3 light_pos;
	float padding;
};

struct vs_in	// vertex shader in
{
	float3 position_local : POSITION;
	float3 position_normal : NORMAL;
	float2 uv : TEX;
};

struct vs_out	// vertex shader out
{
	float4 position_clip : SV_POSITION;
	float3 position_normal : S_NORMAL;
	float2 uv : TEX;
	float3 real_pos : REAL_POS;
};

Texture2D my_texture2d;
SamplerState my_sampler_state;

// entry point for vertex shader
vs_out vs_main(vs_in input)
{
	vs_out output = (vs_out)0;	// clear memory

	output.position_clip = mul(mov_mat, float4(input.position_local, 1.0f));
	//output.position_clip = float4(input.position_local, 1.0);
	output.uv = input.uv;
	output.real_pos = mul(mov_mat, float4(input.position_local, 1.0f));
	output.position_normal = input.position_normal;


	return output;
}

// entry point for pixel shader
float4 ps_main(vs_out input) : SV_TARGET
{
	//float3 normal = input.position_normal;
	float3 normal = input.position_normal;

	float3 material_color = float3(0.f, 0.f, 0.f);	// material/reflection color

	// ambient reflection
	float3 light_ambient = float3(0.2f, 0.2f, 0.2f);
	float3 ambient_reflection = light_ambient * material_color;

	// diffuse reflection

	// get light direction for this fragment
	float3 light_direction = normalize(light_pos - input.real_pos);	// using REAL_POS because apparently "position_clip" is (1, 1, 1)???
		
	float dot_p = dot(light_direction, normal);	// these are both pointing OUTWARDS from the screen!
	if (dot_p < 0.f)
		dot_p = 0.f;

	float3 light_density = float3(0.5f, 0.5f, 0.5f);
	float3 light_color = float3(1.f, 1.f, 1.f);
	float3 diffused_reflection = light_density * light_color * dot_p;

	// specular reflection

	// get the reflected light! we need incident light (light that strikes a surface) and then a reflected light!
	// normal phong specular
	float3 reflected_light = reflect(-light_direction, normal);
	float3 vec_to_viewer = float3(0.f, 0.f, 0.0f) - input.real_pos;

	float3 spec = float3(1.f, 1.f, 1.f);
	float3 intensity = float3(1.f, 1.f, 1.f);

	//float3 specular_reflection = spec * intensity * pow(dot(reflected_light, vec_to_viewer), 1000);

	// blinn phong specular
	float3 h = normalize(vec_to_viewer - (-light_direction));
	float3 specular_reflection = spec * intensity * pow(dot(h, normal), 1000);


	float4 final_color = float4(ambient_reflection + diffused_reflection + specular_reflection, 1.0f);




	return final_color * my_texture2d.Sample(my_sampler_state, input.uv);


	// for point light
	//return final_color;	// returning RGBA color

	// for texture
	//return my_texture2d.Sample(my_sampler_state, input.uv);
}
