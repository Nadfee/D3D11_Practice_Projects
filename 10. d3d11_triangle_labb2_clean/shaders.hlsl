// Namn: Nadhif Ginola
// Programkod: PAASP18

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
	vs_out output = (vs_out)0;

	output.position_clip = mul(float4(input.position_local, 1.0f), mov_mat);
	output.position_normal = input.position_normal;
	output.uv = input.uv;
	output.real_pos = mul(float4(input.position_local, 1.0f), mov_mat);

	return output;
}

// entry point for pixel shader
float4 ps_main(vs_out input) : SV_TARGET
{
	int height;
	int width;

my_texture2d.GetDimensions(width, height);

if (input.position_clip.y <= (float)height / 2.f) return float4(1.f, 0.f, 0.f, 1.f);
else return float4(0.f, 0.f, 1.f, 1.f);



	/* Ambient Light */
	float3 material_color = float3(0.f, 0.f, 0.f);		// material/reflection color (Black)
	float3 light_ambient = float3(0.2f, 0.2f, 0.2f);		// intensity
	float3 ambient_reflection = light_ambient * material_color;

	/* Diffused Light */

	// Get light direction for this fragment
	float3 light_direction = normalize(light_pos - input.real_pos);

	float dot_p = dot(light_direction, input.position_normal);
	if (dot_p < 0.f)
		dot_p = 0.f;

	float3 light_density = float3(0.5f, 0.5f, 0.5f);	
	float3 light_color = float3(1.f, 1.f, 1.f);			// white
	float3 diffused_reflection = light_density * light_color * dot_p;

	/* Blinn Phong Specular Light */
	float3 reflected_light = reflect(light_direction, input.position_normal);
	float3 vec_to_viewer = float3(input.real_pos.x, input.real_pos.y, 0.0f) - input.real_pos;

	float3 spec = float3(1.f, 1.f, 1.f);
	float3 intensity = float3(1.f, 1.f, 1.f);

	float3 h = normalize(vec_to_viewer - (-light_direction));

	float dot_hn = dot(light_direction, input.position_normal);
	if (dot_hn < 0.f)
		dot_hn = 0.f;

	float3 specular_reflection = spec * intensity * pow(dot_hn, 1000);

	float4 final_light = float4(ambient_reflection + diffused_reflection + specular_reflection, 1.0f);

	return final_light * my_texture2d.Sample(my_sampler_state, input.uv);
}
