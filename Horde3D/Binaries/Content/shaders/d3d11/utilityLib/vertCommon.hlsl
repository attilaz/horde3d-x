// *************************************************************************************************
// Horde3D Shader Utility Library
// --------------------------------------
//		- Common functions -
//
// Copyright (C) 2006-2011 Nicolas Schulz
//
// You may use the following code in projects based on the Horde3D graphics engine.
//
// *************************************************************************************************

float4x4 viewMat;
float4x4 worldMat;
float3x3 worldNormalMat;


float4 calcWorldPos( const float4 pos )
{
	return mul(worldMat, pos);
}

float4 calcViewPos( const float4 pos )
{
	return mul(viewMat, pos);
}

float3 calcWorldVec( const float3 vec )
{
	return mul(worldNormalMat, vec);
}

float3x3 calcTanToWorldMat( const float3 tangent, const float3 bitangent, const float3 normal )
{
	return float3x3( tangent, bitangent, normal );
}

float3 calcTanVec( const float3 vec, const float3 tangent, const float3 bitangent, const float3 normal )
{
	float3 v;
	v.x = dot( vec, tangent );
	v.y = dot( vec, bitangent );
	v.z = dot( vec, normal );
	return v;
}