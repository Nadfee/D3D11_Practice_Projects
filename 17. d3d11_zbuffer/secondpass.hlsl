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

float lerp();

struct VSInput
{
	float3 positionLS : POSITION;
	float3 positionN : NORMAL;
	float2 uv : TEX;
};

struct VSOutput
{
	float4 positionCS : SV_POSITION;
	float3 positionN : NORMAL;
	float4 positionLCS : REALPOS;
	float3 positionLWS : LWORLDPSPACE;
	float2 uv : TEX;
};

VSOutput vs_main(VSInput input)
{
	VSOutput output = (VSOutput)0;
	output.positionCS = mul(float4(input.positionLS, 1.0f), m_WorldViewProj);
	output.positionLCS = mul(float4(input.positionLS, 1.0f), m_LWorldViewProj);
	output.positionN = input.positionN;
	output.uv = input.uv;
	output.positionLWS = mul(float4(input.positionLS, 1.0f), m_World).xyz;

	return output;
}

Texture2D depthMap : register(t0);
SamplerState my_sampler : register(s0);

float4 ps_main(VSOutput input) : SV_TARGET
{
	static const float SMAP_SIZE = 1024.0f;
	static const float SHADOW_EPSILON = 0.0005f;		// 0.001f fixes artifacting when up close (?)

	// get our vertex position relative to the light in NDC! (we were in CS before)
	input.positionLCS.xy /= input.positionLCS.w;
	
	// get the depth of the pixel relative to the light!
	float depth = input.positionLCS.z / input.positionLCS.w;

	// get the uv coordinates for our shadowmap
	// because NDC is [-1, 1] in x, y we need to make it go from [0, 1]
	float2 smUv = float2(0.5f * input.positionLCS.x + 0.5f, -0.5f * input.positionLCS.y + 0.5f);
	float shadowMapDepth = depthMap.Sample(my_sampler, smUv).r;

	// Frank Luna

	// Fix 3x3 point filter for softer shadows (if you zoom in you can see grey)

	// 2x2 point filter
	float dx = 1.0f / SMAP_SIZE; // size of one texture sample in the shadowmap (width==height)
	float s0 = (depthMap.Sample(my_sampler, smUv).r + SHADOW_EPSILON < depth) ? 0.0f : 1.0f;
	float s1 = (depthMap.Sample(my_sampler, smUv + float2(dx, 0.0f)).r + SHADOW_EPSILON < depth) ? 0.0f : 1.0f;
	float s2 = (depthMap.Sample(my_sampler, smUv + float2(0.0f, dx)).r + SHADOW_EPSILON < depth) ? 0.0f : 1.0f;
	float s3 = (depthMap.Sample(my_sampler, smUv + float2(dx, dx)).r + SHADOW_EPSILON < depth) ? 0.0f : 1.0f;

	// Transform shadow map UV coord totexel space
	float2 texelPos = smUv * SMAP_SIZE;
	
	float2 lerps = frac(texelPos);

	float shadowCoeff = lerp(lerp(s0, s1, lerps.x), lerp(s2, s3, lerps.x), lerps.y);

	float3 L = normalize(lightPos - input.positionLWS);
	float diffused = dot(normalize(input.positionN), L);

	float3 litColor =  diffused * float3(1.f, 1.f, 1.f) * shadowCoeff;

	return float4(litColor, 1.f);

	//// Shadow Epsilon to avoid self shadowing! (essentially comparing two depth values that are "the same" (the surface is lit)
	//if (shadowMapDepth + SHADOW_EPSILON < depth)
	//	return float4(0.f, 0.f, 0.f, 1.f);
	//else
	//	return float4(shadowMapDepth, shadowMapDepth, shadowMapDepth, 1.f) /** diffused*/;
}