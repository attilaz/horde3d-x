// *************************************************************************************************
// Horde3D Shader Utility Library
// --------------------------------------
//		- Particle functions -
//
// Copyright (C) 2006-2011 Nicolas Schulz
//	D3D11 version 2012-2013 Attila Kocsis
//
// You may use the following code in projects based on the Horde3D graphics engine.
//
// *************************************************************************************************

float4x4 viewMat;
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
	float3 camAxisX = viewMat[0].xyz;
	float3 camAxisY = viewMat[1].xyz;
	
	float2 cornerPos = texCoords - float2( 0.5, 0.5 );
	
	// Apply rotation
	float s = sin( parSizeAndRotArray[index].y );
	float c = cos( parSizeAndRotArray[index].y );
	cornerPos = mul( float2x2( c, -s, s, c ) , cornerPos);
	
	return parPosArray[index] + (camAxisX * cornerPos.x + camAxisY * cornerPos.y) * parSizeAndRotArray[index].x;
}