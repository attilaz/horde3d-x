// *************************************************************************************************
// Horde3D Shader Utility Library
// --------------------------------------
//		- Deferred shading functions -
//
// Copyright (C) 2006-2011 Nicolas Schulz
//	D3D11 version 2012-2013 Attila Kocsis
//
// You may use the following code in projects based on the Horde3D graphics engine.
//
// *************************************************************************************************

struct fragDefferedBuffer
{
	float4 target0 : SV_TARGET0;
	float4 target1 : SV_TARGET1;
	float4 target2 : SV_TARGET2;
	float4 target3 : SV_TARGET3;
};

void setMatID( inout fragDefferedBuffer fragData, const float id ) { fragData.target0.a = id; }
void setPos( inout fragDefferedBuffer fragData, const float3 pos ) { fragData.target0.rgb = pos; }
void setNormal( inout fragDefferedBuffer fragData, const float3 normal ) { fragData.target1.rgb = normal; }
void setAlbedo( inout fragDefferedBuffer fragData, const float3 albedo ) { fragData.target2.rgb = albedo; }
void setSpecParams( inout fragDefferedBuffer fragData, const float3 specCol, const float gloss ) { fragData.target3.rgb = specCol; fragData.target3.a = gloss; }
