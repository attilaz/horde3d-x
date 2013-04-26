[[FX]]

// Samplers
sampler2D albedoMap = sampler_state
{
	Filter = Bilinear;
	MaxAnisotropy = 1;
};

// Contexts
/*context SHADOWMAP
{
	VertexShader = compile GLSL VS_SHADOWMAP;
	PixelShader = compile GLSL FS_SHADOWMAP;
}*/

context TRANSLUCENT
{
	VertexShader = compile GLSL VS_TRANSLUCENT;
	PixelShader = compile GLSL FS_TRANSLUCENT;
	
	ZWriteEnable = false;
	BlendMode = AddBlended;
}


[[VS_SHADOWMAP]]
// =================================================================================================

#include "shaders/d3d11/utilityLib/vertParticle.glsl"

uniform float4x4 viewProjMat;
uniform float4 lightPos;
attribute float2 texCoords0;
varying float dist;

void main(void)
{
	float4 pos = float4( calcParticlePos( texCoords0 ), 1 );
	dist = length( lightPos.xyz - pos.xyz ) / lightPos.w;
	
	gl_Position = viewProjMat * pos;
}
				
				
[[FS_SHADOWMAP]]
// =================================================================================================

uniform float shadowBias;
varying float dist;

void main( void )
{
	gl_FragDepth = dist + shadowBias;
}


[[VS_TRANSLUCENT]]
// =================================================================================================

#include "shaders/d3d11/utilityLib/vertParticle.hlsl"

float4x4 viewProjMat;

struct VS_INPUT
{
	float2 texCoords0 : TEXCOORD0;
	float parIdx : BLENDINDICES0;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;

	float4 color : COLOR0;
	float2 texCoords : TEXCOORD0;
};

VS_OUTPUT main( VS_INPUT In )
{
	VS_OUTPUT Out = (VS_OUTPUT)0;
	Out.color = getParticleColor( In.parIdx );
	Out.texCoords = float2( In.texCoords0.x, -In.texCoords0.y );
	Out.position = mul( viewProjMat, float4( calcParticlePos( In.parIdx, In.texCoords0 ), 1 ) );
	
	return Out;
}


[[FS_TRANSLUCENT]]
// =================================================================================================

struct VS_OUTPUT
{
	float4 position : SV_POSITION;

	float4 color : COLOR0;
	float2 texCoords : TEXCOORD0;
};

Texture2D texture_albedoMap;
SamplerState sampler_albedoMap;

float4 main( VS_OUTPUT In ) : SV_Target
{
	float4 albedo = texture_albedoMap.Sample( sampler_albedoMap, In.texCoords );
	
	return albedo * In.color;
}
