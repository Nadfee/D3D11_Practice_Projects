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
	float4 positionCSS : RPOS;	// used in rastertek tut 35 (depth buffer)
};

vs_out vs_main(vs_in input)
{
	vs_out output = (vs_out)0;

	output.positionCS = mul(float4(input.positionLS, 1.0f), m_WorldViewProj);
	output.positionN = input.positionN;
	output.uv = input.uv;
	//output.positionWS = mul(float4(input.positionLS, 1.0f), m_World);
	output.positionCSS = output.positionCS;

	return output;
}

float4 ps_main(vs_out input) : SV_TARGET
{
	///* Ambient Light */
	//float3 material_color = float3(0.3f, 0.5f, 0.7f);		// material/reflection color (Black)
	//float3 light_ambient = float3(0.2f, 0.2f, 0.2f);		// intensity
	//float3 ambient_reflection = light_ambient * material_color;

	///* Diffused Light */

	//// Get light direction for this fragment
	//float3 light_direction = normalize(input.positionWS.xyz - lightPos);

	//float dot_p = dot(-light_direction, normalize(input.positionN));
	//if (dot_p < 0.f)
	//	dot_p = 0.f;

	//float3 light_density = float3(0.5f, 0.5f, 0.5f);	
	//float3 light_color = float3(1.f, 1.f, 1.f);			// white
	//float3 diffused_reflection = light_density * light_color * dot_p;

	///* Blinn Phong Specular Light */
	//float3 vec_to_viewer = cameraPos - input.positionWS.xyz;

	//float3 spec = float3(1.f, 1.f, 1.f);
	//float3 intensity = float3(1.f, 1.f, 1.f);

	//float3 h = normalize(vec_to_viewer - light_direction);

	//float dot_hn = dot(h, normalize(input.positionN));
	//if (dot_hn < 0.f)
	//	dot_hn = 0.f;
 //
	//if (dot(-light_direction, input.positionN) < 0.f)	// Light is facing towards the same direction as normal
	//	dot_hn = 0.f;										// So not actually hitting surface (Same as for Diffused in line 56/57

	//float3 specular_reflection = spec * intensity * pow(dot_hn, 1000);

	//float4 final_light = float4(ambient_reflection + diffused_reflection + specular_reflection, 1.0f);
	//return final_light;


	// Visualization of how the Depth Buffer (Depth Value) is NOT linear!
	// Depth Buffer is the depth [0, 1] from the Near Plane to the Far Plane!
	// If you run this code, you will see black to red shade that have precision
	// but it it quickly falls off and becomes '99.99% red'. 
	// This is because 90% of the floating point values occur in the first 10% of the depth buffer close to the Near Clip Plane!
	// The remaining 10% (from 0.9f to 1.0f takes up the last 90% of the buffer)

	float depthValue;
	float4 color;

	depthValue = input.positionCSS.z / input.positionCSS.w;	// in NDC now

	//if (depthValue < 0.9f)
	//{
	//	color = float4(depthValue, 0.f, 0.f, 1.f);
	//}

	//if (depthValue > 0.9f)
	//{
	//	color = float4(1.0f, 0.f, 0.f, 1.f);
	//}

	//if (depthValue > 0.925f)
	//{
	//	color = float4(0.f, 0.f, 1.f, 1.f);
	//}

	// See how quickly it becomes '99.99% red' at halfway mark!
	color = float4(depthValue, 0.f, 0.f, 1.f);

	// Compare with this, this makes sure to interpolate from 0 to 1 with the z coordinates [0, 3] (size of floor)
	//color = float4(input.positionWS.z / 3.f, 0.f, 0.f, 1.f);

	

	
		

	return color;

}
