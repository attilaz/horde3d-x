// *************************************************************************************************
// Horde3D Shader Utility Library
// --------------------------------------
//		- Particle functions -
//
// Copyright (C) 2006-2011 Nicolas Schulz
//
// You may use the following code in projects based on the Horde3D graphics engine.
//
// *************************************************************************************************

float4x4 viewMatInv;
float3 parPosArray[64];
float2 parSizeAndRotArray[64];
float4 parColorArray[64];


float4 getParticleColor(float parIdx)
{
	return parColorArray[int( parIdx )];
}

float3 calcParticlePos( float parIdx, const float2 texCoords )
{
	int index = int( parIdx );
	float3 camAxisX = viewMatInv[0].xyz;
	float3 camAxisY = viewMatInv[1].xyz;
	
	float2 cornerPos = texCoords - float2( 0.5, 0.5 );
	
	// Apply rotation
	float s = sin( parSizeAndRotArray[index].y );
	float c = cos( parSizeAndRotArray[index].y );
	cornerPos = mul( float2x2( c, -s, s, c ) , cornerPos);
	
	return parPosArray[index] + (camAxisX * cornerPos.x + camAxisY * cornerPos.y) * parSizeAndRotArray[index].x;
}