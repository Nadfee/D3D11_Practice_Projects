cbuffer Movement : register(b1)
{
	matrix m_WorldViewProj;
	matrix m_World;
	matrix m_View;
	matrix m_Proj;
	matrix m_WorldView;
	float4 samples[64];
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


Texture2D posVS_text : register(t0);
Texture2D norVS_text : register(t1);
Texture2D noiseText : register(t2);
Texture2D depthText : register(t3);		// Not using
SamplerState my_sampler : register(s0);
SamplerState noiseSampler : register(s1);



vs_out vs_main(vs_in input)
{
	vs_out output = (vs_out)0;

	output.positionCS = float4(input.positionLS, 1.0f);
	output.uv = input.uv;

	return output;
}

float3 ps_main(vs_out input) : SV_TARGET0
{
	//return norVS_text.Sample(my_sampler, input.uv).xyz * 2.0 - 1.0;;	// Correct normals! In Viewspace. || Y+ : Gives Green || X+ : Gives Red || Z+ : Gives Blue. Remember that everything is relative camera now! (Camera is origin)
	//return float3(posVS_text.Sample(my_sampler, input.uv).x, posVS_text.Sample(my_sampler, input.uv).y, 0.f);	// Correct viewspace position! Middle should be Black and a meeting point for all colors, Right side of middle should be red and top side of middle should be green (Camera relative again)

	int width = 0;
	int height = 0;

	//posVS_text.GetDimensions(width, height);
	//float2 noiseScale = float2((float)width / 8.f, (float)height / 8.f);

	float2 noiseScale = float2(1424.f / 8.f, 720.f / 8.f);
	float radius = 0.2f;	// This radius saves it all from artifacting! Try with higher value :)
	// https://mtnphil.wordpress.com/2013/06/26/know-your-ssao-artifacts/
	// There is a diagram talking about Radius artifacting! 
	// and a picture! take a look.
	// It also makes sense that 0.25f would work here. The "radius" in this is a radius in viewspace;
	// meaning it still hasnt been squished (after projection) and has the dimensions of world space objects
	// 0.25f in radius meaning 0.25f in relation to the triangle's lengths where "radius = 1.f" is actually 
	// a pretty large half-hemisphere.
	// If you think about going up close to a surface, certain sample points may be behind the camera
	// and it may have a Depth value of something we dont know?
	// remember that we essentially "project a ray" from the camera to the sample point, if it points to something
	// behind the camera then I'm not totally sure what I'm getting back.

	// Background going crazy not solved (only if occlusion is showed) - Perhaps this doesn't matter because with Phong lighting, background gets no lighting anyways
	// and real objects have their correct occlusion factor for each position.
	if (posVS_text.Sample(my_sampler, input.uv).x == 0.f && posVS_text.Sample(my_sampler, input.uv).y == 0.f && posVS_text.Sample(my_sampler, input.uv).z == 0.f)
		return float3(1.f, 1.f, 1.f);

	// John Chapman Tutorial
	float3 origin = posVS_text.Sample(my_sampler, input.uv).xyz;
	float3 normal = norVS_text.Sample(my_sampler, input.uv).xyz;
	normal = normalize(normal);
	
	float3 rvec = noiseText.Sample(noiseSampler, input.uv * noiseScale).xyz;
	float3 tangent = normalize(rvec - normal * dot(rvec, normal));
	float3 bitangent = cross(normal, tangent);

	// OpenGL (vector x matrix)
	//float3 row1 = float3(tangent.x, bitangent.x, normal.x);
	//float3 row2 = float3(tangent.y, bitangent.y, normal.y);
	//float3 row3 = float3(tangent.z, bitangent.z, normal.z);
	//float3x3 tbn = float3x3(row1, row2, row3);

	
	float3x3 tbn = float3x3(tangent, bitangent, normal);

	float occlusion = 0.0f;
	for (int i = 0; i < 64; i++)
	{
		// Get sample position in Viewspace. This takes the samples from a "local" space all the way to Viewspace albeit with the origin at (0, 0, 0) (in viewspace)
		// but with the orientation so that 'z' is the normal of the current fragment (from buffer). This "orientation so that 'z' is the normal of the current fragment" is called a Tangent space!
		// This is why we need "+ origin" below

		
		//float3 mSample = mul(tbn, samples[i].xyz);		// Note that the mul is in this order because the TBN is created for this order (tutorials)
		// We can change the order of mul (so it is in typical vector first but you will have to transpose the tbn)
		
		float3 mSample = mul(samples[i].xyz, tbn);
		mSample = mSample * radius + origin;				
		// mSample's depth will be used. It is the depth from the camera to the sample point as if theres an object there!

		// Project sample position (to find texture coordinates)
		float4 offset = float4(mSample, 1.f);
		offset = mul(offset, m_Proj);				// get clip space pos
		offset.xy /= offset.w;						// get ndc
		offset.xy = offset.xy * 0.5f + 0.5f;		// scale to texture coord [0, 1]
		offset.y = 1.f - offset.y;					// fix our texture coordinate - read below
		
		// This below applies to offset throughout the whole process above since the Viewspace defines the center in the middle.
		// Note that "offset" stems from "mSample" which stems from "origin". As origin is in viewspace, our camera is the origin of our space. This means that the middle of
		// the screen should be "black" (0, 0), top left should be "green" (-a, 1) bottom right should be "red" (1, -a), top right should be "yellow" (1, 1)
		// This is if you return float3(offset.x, offset.y, 0.f) and show it to the screen!
		// The comments above ultimately means that the Y texture coordinates need to be inverted as the very bottom left of the screen is "black" (-a, -a)

		// "You probably noticed that the texture is flipped upside-down! This happens because OpenGL expects 
		// the 0.0 coordinate on the y-axis to be on the bottom side of the image, but images usually have 0.0 at the top of the y-axis."
		// -LearnOpenGL
		// Hence why there is no Y-axis flips in tutorials! Because in this case, its expected the black on the bottom left and not upper left like in D3D11

		// Get actual sample depth (Viewspace depth) (for actual vertices/objects in scene)
		float sampleDepth = posVS_text.Sample(my_sampler, offset.xy).z;

		
		// THE CULPRIT WAS THE RANGE CHECK!!!! IF RADIUS IS FOR EXAMPLE 1.F IT BREAKS AND ARTIFACTS WHEN CLOSE
		// As long as Radius is not 1.f or more, then it is fine. It artifacts more the higher radius!!!
		float rangeCheck = abs(origin.z - sampleDepth) < radius ? 1.0 : 0.0;	

		// Finally check if the sample point depth is closer than the actual depth for that position (along the same "line")
		// If sample point depth is closer, there is nothing blocking it since the actual object is behind the point, thus the point is NOT occluded (0.0)
		// If actual depth (sampleDepth) is closer or equal, then there is blocking the sample point, which means that the sample point must be inside or behind the vertex, thus the point IS occluded (1.0)
		occlusion += (sampleDepth <= mSample.z ? 1.0 : 0.0) * rangeCheck;
	}
	occlusion = 1.0 - (occlusion / 64.f);
	return float3(occlusion, occlusion, occlusion);


	// This will give the exact same results as above. Both works
	// OpenGL Tutorial
	//float3 fragPos = posVS_text.Sample(my_sampler, input.uv).xyz;		
	//float3 normal = normalize(norVS_text.Sample(my_sampler, input.uv).xyz);
	//float3 randomVec = noiseText.Sample(noiseSampler, input.uv * noiseScale).xyz;

	//
	//float3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	//float3 bitangent = cross(normal, tangent);

	//float3 row1 = float3(tangent.x, bitangent.x, normal.x);
	//float3 row2 = float3(tangent.y, bitangent.y, normal.y);
	//float3 row3 = float3(tangent.z, bitangent.z, normal.z);
	//float3x3 TBN = float3x3(row1, row2, row3);

	//float occlusion = 0.f;
	//for (int i = 0; i < 64; i++)
	//{
	//	// Get sample position in viewspace
	//	float3 mySample = mul(TBN, samples[i].xyz);
	//	mySample = fragPos + (mySample * radius);

	//	// We want to get the texture coordinates for the sample point so that we can sample the correct viewspace position from our texture
	//	float4 offset = float4(mySample, 1.f);										// -- sample point in viewspace
	//	offset = mul(offset, m_Proj);				// view to clip					// -- sample point in clipspace
	//	offset.xy /= offset.w;						// clip to ndc (persp div)		// -- sample point in ndc [-1, 1] in xy
	//	offset.xy = offset.xy * 0.5f + 0.5f;		// transform to range [0, 1]	// -- sample point in texcoord																
	//	offset.y = 1.f - offset.y;					// Now the Texture is just upside down! But its correct depth! (But why?)

	//	// Finally sample the "real" depth on the sample location
	//	float sampleDepth = posVS_text.Sample(my_sampler, offset.xy).z;	
	//		
	//	// Without the range check, the edges of anything will be occluded (because neighbors may be just empty space and THAT has maximum depth value! 
	//	// E.g Occluder Point (sample.z) WILL be larger (further away) than the Ocludee (sampleDepth)
	//	float rangeCheck = abs(fragPos.z - sampleDepth) < 0.2f ? 1.0 : 0.0;
	//		
	//	// If the "real" depth of where the sample point was suppose to be is less or equal, then there is an object there, thus meanign the sample point should be occluded (inside a surface)
	//	occlusion += (sampleDepth <= mySample.z ? 1.0f : 0.0f) * rangeCheck;

	//}
	//float finalOcclFactor = 1.f - (occlusion / 64.f);
	//return float3(finalOcclFactor, finalOcclFactor, finalOcclFactor);

}