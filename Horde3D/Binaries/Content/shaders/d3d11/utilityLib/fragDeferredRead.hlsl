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

Texture2D texture_gbuf0;
SamplerState sampler_gbuf0;

Texture2D texture_gbuf1;
SamplerState sampler_gbuf1;

Texture2D texture_gbuf2;
SamplerState sampler_gbuf2;

Texture2D texture_gbuf3;
SamplerState sampler_gbuf3;

float getMatID( const float2 coord ) { return texture_gbuf0.Sample( sampler_gbuf0, coord ).a; }
float3 getPos( const float2 coord ) { return texture_gbuf0.Sample( sampler_gbuf0, coord ).rgb; }
float3 getNormal( const float2 coord ) { return texture_gbuf1.Sample( sampler_gbuf1, coord ).rgb; }
float3 getAlbedo( const float2 coord ) { return texture_gbuf2.Sample( sampler_gbuf2, coord ).rgb; }
float4 getSpecParams( const float2 coord ) { return texture_gbuf3.Sample( sampler_gbuf3, coord ).rgba; }
