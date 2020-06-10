cbuffer Light : register(b0)
{
	float3 lightPos		: LIGHTPOS;
	float padding1;
	float3 cameraPos	: CAMERAPOS;
	float padding2;
};

struct vs_in
{
	float3 positionLS : POSITION;
	float2 uv : TEXCOORD;
};

struct vs_out
{
	float4 positionCS : SV_POSITION;
	float2 uv : TEXCOORD;
};

vs_out vs_main(vs_in input)
{
	vs_out output = (vs_out)0;

	output.positionCS = float4(input.positionLS, 1.f);
	output.uv = input.uv;

	return output;
}

// accessing g buffers via srv
Texture2D pos_text : register(t0);
Texture2D nor_text : register(t1);
Texture2D col_text : register(t2);
SamplerState my_sampler : register(s0);

float4 ps_main(vs_out input) : SV_TARGET
{
	/* Ambient Light */
	float3 material_color = float3(0.f, 0.f, 0.f);		// material/reflection color (Black)
	float3 light_ambient = float3(0.2f, 0.2f, 0.2f);		// intensity
	float3 ambient_reflection = light_ambient * material_color;

	/* Diffused Light */

	// Get light direction for this fragment
	float3 light_direction = normalize(pos_text.Sample(my_sampler, input.uv).xyz - lightPos);

	float dot_p = dot(-light_direction, normalize(nor_text.Sample(my_sampler, input.uv).xyz));
	if (dot_p < 0.f)
		dot_p = 0.f;

	float3 light_density = float3(0.5f, 0.5f, 0.5f);
	float3 light_color = float3(1.f, 1.f, 1.f);			// white
	float3 diffused_reflection = light_density * light_color * dot_p;

	/* Blinn Phong Specular Light */
	float3 vec_to_viewer = cameraPos - pos_text.Sample(my_sampler, input.uv).xyz;

	float3 spec = float3(1.f, 1.f, 1.f);
	float3 intensity = float3(1.f, 1.f, 1.f);

	float3 h = normalize(vec_to_viewer - light_direction);

	float dot_hn = dot(h, nor_text.Sample(my_sampler, input.uv).xyz);
	if (dot_hn < 0.f)
		dot_hn = 0.f;

	if (dot(-light_direction, nor_text.Sample(my_sampler, input.uv).xyz) < 0.f)
		dot_hn = 0.f;

	float3 specular_reflection = spec * intensity * pow(dot_hn, 1000);

	float4 final_light = float4(ambient_reflection + diffused_reflection + specular_reflection, 1.0f);

	//return pos_text.Sample(my_sampler, input.uv);
	return final_light;
	//return float4(0.f, 0.f, pos_text.Sample(my_sampler, input.uv).z, 1.f);
}