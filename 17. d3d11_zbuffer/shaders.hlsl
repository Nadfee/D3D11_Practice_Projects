cbuffer Movement : register(b0)
{
	matrix m_WorldViewProj;
	matrix m_World;
	matrix m_LWorldViewProj;
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
	float2 uv : TEX;
};

//struct vs_out
//{
//	float4 positionCS : SV_POSITION;
//	float4 positionCSS : REALPOS_POSITION;
//};

float4 vs_main(vs_in input) : SV_POSITION
{
	return mul(float4(input.positionLS, 1.0f), m_LWorldViewProj);
}


//vs_out vs_main(vs_in input)
//{
	//vs_out output = (vs_out)0;
	//output.positionCS = mul(float4(input.positionLS, 1.0f), m_LWorldViewProj);
	//output.positionCSS = mul(float4(input.positionLS, 1.0f), m_LWorldViewProj);

	//return output;	// Render vertices just from light persp to populate depth buffer (texture attached to our depth stencil view)
//}

//float4 ps_main(vs_out input) : SV_TARGET
//{
//	input.positionCSS.z /= input.positionCSS.w;
//	
//	//return float4(1.f, 1.f, 1.f, 1.f);
//	return float4(input.positionCSS.z, input.positionCSS.z, input.positionCSS.z, 1.f);
//}

void ps_main()
{

}
