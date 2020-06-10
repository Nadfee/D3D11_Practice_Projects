cbuffer const_buf
{
	float3 position;
	float padding;
};

struct vs_in	// vertex shader in
{
	float3 position_local : POSITION;
	float3 normal : NORMAL;
};

struct vs_out	// vertex shader out
{
	float4 position_clip : SV_POSITION;
	float3 normal : NORMAL;
	float3 real_pos : REALPOS;
};

// entry point for vertex shader
vs_out vs_main(vs_in input)
{
	vs_out output = (vs_out)0;	// clear memory
	output.position_clip = float4(input.position_local, 1.0);
	output.normal = input.normal;
	output.real_pos = input.position_local;
	return output;
}

// entry point for pixel shader
float4 ps_main(vs_out input) : SV_TARGET
{
	float3 material_color = float3(1.f, 0.f, 0.f);	// material/reflection color

	// ambient reflection
	float3 light_ambient = float3(0.2f, 0.2f, 0.2f);
	float3 ambient_reflection = light_ambient * material_color;

	// diffuse reflection

	// get light direction for this fragment
	float3 light_direction = normalize(position - input.real_pos);	// using REAL_POS because apparently "position_clip" is (1, 1, 1)???

	float dot_p = dot(light_direction, input.normal);	// these are both pointing OUTWARDS from the screen!
	if (dot_p < 0.f)
		dot_p = 0.f;

	float3 light_density = float3(0.7f, 0.7f, 0.7f);
	float3 light_color = float3(0.f, 1.f, 0.f);
	float3 diffused_reflection = light_density * light_color * dot_p;

	// specular reflection

	// get the reflected light! we need incident light (light that strikes a surface) and then a reflected light!
	float3 reflected_light = reflect(-light_direction, input.normal);
	float3 vec_to_viewer = float3(input.real_pos.x, input.real_pos.y, 0.0f) - input.real_pos;

	float3 spec = float3(1.f, 1.f, 1.f);
	float3 intensity = float3(1.f, 1.f, 1.f);

	//float3 specular_reflection = spec * intensity * pow(dot(reflected_light, vec_to_viewer), 1000);

	// blinn phong
	float3 h = normalize(vec_to_viewer - (-light_direction));
	float3 specular_reflection = spec * intensity * pow(dot(h, input.normal), 3000);


	float4 final_color = float4(ambient_reflection + diffused_reflection + specular_reflection, 1.0f);

	return final_color;	// returning RGBA color
	//return float4(input.real_pos, 1.0f);
	//return float4(light_direction, 1.0f);
}
