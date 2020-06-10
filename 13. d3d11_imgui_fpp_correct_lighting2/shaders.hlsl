cbuffer Movement : register(b0)
{
	matrix m_WorldViewProj;
	matrix m_World;
};

cbuffer Light : register(b1)
{
	float3 lightPos;
	float padding;
	float3 cameraPos;
	float padding2;
};

Texture2D tTexture : register(t0);
SamplerState sPoint : register(s0);

struct vs_in
{
	float3 positionLS : POSITION;
	float3 positionN : NORMAL;
	float2 uv : TEXCOORD;
};

struct vs_out
{
	float4 positionCS : SV_POSITION;
	float3 positionN : S_NORMAL;
	float2 uv : TEXCOORD;
	float3 positionWS : RPOS;
};

vs_out vs_main(vs_in input)
{
	vs_out output = (vs_out)0;

	output.positionCS = mul(float4(input.positionLS, 1.0f), m_WorldViewProj);
	output.positionN = normalize(mul(float4(input.positionN, 0.f), m_World));
	output.uv = input.uv;
	output.positionWS = mul(float4(input.positionLS, 1.0f), m_World);

	return output;
}

float4 ps_main(vs_out input) : SV_TARGET
{
	//return tTexture.Sample(sPoint, input.uv);

	//return float4(input.positionN.xyz, 1.f);

	//return float4(input.uv, 0.f, 1.f);

	//return float4(input.positionWS.xyz, 1.f);

	//float x = input.uv.x - 0.5f;
	//float y = input.uv.y - 0.5f;

	//if (x * x + y * y < 0.1f)
	//{
	//	return float4(0.f, 0.f, 0.f, 1.f);
	//}
	//else if (input.uv.x <= 0.5f)
	//{
	//	return float4(1.f, 0.f, 0.f, 1.f);
	//}
	//else if (input.uv.x > 0.5f)
	//{
	//	return float4(0.f, 1.f, 0.f, 1.f);
	//}



	/* Ambient Light */
	float3 material_color = float3(0.3f, 0.5f, 0.7f);		// material/reflection color (Black)
	float3 light_ambient = float3(0.2f, 0.2f, 0.2f);		// intensity
	float3 ambient_reflection = light_ambient * material_color;

	/* Diffused Light */

	// Get light direction for this fragment
	float3 light_direction = normalize(lightPos - input.positionWS);

	float dot_p = dot(light_direction, normalize(input.positionN));
	if (dot_p < 0.f)
		dot_p = 0.f;

	float3 light_density = float3(0.5f, 0.5f, 0.5f);	
	float3 light_color = float3(1.f, 1.f, 1.f);			// white
	float3 diffused_reflection = light_density * light_color * dot_p;

	/* Blinn Phong Specular Light */
	float3 reflected_light = reflect(light_direction, input.positionN);
	float3 vec_to_viewer = cameraPos - input.positionWS;

	float3 spec = float3(1.f, 1.f, 1.f);
	float3 intensity = float3(1.f, 1.f, 1.f);

	float3 h = normalize(vec_to_viewer - light_direction);

	float dot_hn = dot(h, normalize(input.positionN));
	if (dot_hn < 0.f)
		dot_hn = 0.f;
 
	if (dot(light_direction, input.positionN) < 0.f)	// Light is facing towards the same direction as normal
		dot_hn = 0.f;										// So not actually hitting surface (Same as for Diffused in line 56/57

	float3 specular_reflection = spec * intensity * pow(dot_hn, 1000);

	float4 final_light = float4(ambient_reflection + diffused_reflection /*+ specular_reflection*/, 1.0f);


	return final_light;
}
