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
	output.positionCS = float4(input.positionLS, 1.0f);
	output.uv = input.uv;

	return output;
}

struct ps_out
{
	float occlusion : SV_TARGET0;
};

Texture2D occlTex : register(t0);
SamplerState mySampler : register(s0);

ps_out ps_main(vs_out input)
{
	ps_out output = (ps_out)0;

	int width = 0;
	int height = 0;
	int uBlurSize = 8;

	occlTex.GetDimensions(width, height);

	// ssao blur 
	float2 texelSize = float2(1.f, 1.f) / float2(width, height);
	float result = 0.0;
	for (int x = -4; x < 4; ++x)		// takes ssao input of the 4x4 (neighbors) and average em out for this current pixel (middle)
	{
		for (int y = -4; y < 4; ++y)
		{
			float2 offset = float2(float(x), float(y)) * texelSize;
			result += occlTex.Sample(mySampler, input.uv + offset).r;
		}
	}
	float FragColor = result / (8.0 * 8.0);

	output.occlusion = FragColor;
	//output.occlusion = occlTex.Sample(mySampler, input.uv).x;

	return output;
}