[[FX]]

// Samplers
samplerCube albedoMap = sampler_state
{
	Address = Clamp;
};

/*
// Contexts
context ATTRIBPASS
{
	VertexShader = compile GLSL VS_GENERAL;
	PixelShader = compile GLSL FS_ATTRIBPASS;
}
*/

context AMBIENT
{
	VertexShader = compile GLSL VS_GENERAL;
	PixelShader = compile GLSL FS_AMBIENT;
}


[[VS_GENERAL]]
// =================================================================================================

#include "shaders/d3d11/utilityLib/vertCommon.hlsl"

float4x4 viewProjMat;
float3 viewerPos;

struct VS_INPUT
{
	float3 vertPos	  : POSITION;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;

	float3 viewVec : TEXCOORD0;
};

VS_OUTPUT main( VS_INPUT In )
{
	VS_OUTPUT Out = (VS_OUTPUT)0;
	
	float4 pos = calcWorldPos( float4( In.vertPos, 1.0 ) );
	Out.viewVec = pos.xyz - viewerPos;
	
	Out.position = mul( viewProjMat, pos);
	
	return Out;
}
				

[[FS_ATTRIBPASS]]
// =================================================================================================

#include "shaders/d3d11/utilityLib/fragDeferredWrite.hlsl"

uniform samplerCube albedoMap;
varying float3 viewVec;

void main( void )
{
	float3 albedo = textureCube( albedoMap, viewVec ).rgb;
	
	// Set fragment material ID to 2, meaning skybox in this case
	setMatID( 2.0 );
	setAlbedo( albedo );
}


[[FS_AMBIENT]]
// =================================================================================================

TextureCube texture_albedoMap;
SamplerState sampler_albedoMap;

struct VS_OUTPUT
{
	float4 position : SV_POSITION;

	float3 viewVec : TEXCOORD0;
};

float3 main( VS_OUTPUT In) : SV_Target
{
	float3 albedo = texture_albedoMap.Sample( sampler_albedoMap, In.viewVec ).rgb;
	
	return albedo;
}
