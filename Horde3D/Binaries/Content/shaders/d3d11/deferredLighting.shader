[[FX]]

// Samplers
sampler2D depthBuf = sampler_state
{
	Address = Clamp;
};

sampler2D gbuf0 = sampler_state
{
	Address = Clamp;
};

sampler2D gbuf1 = sampler_state
{
	Address = Clamp;
};

sampler2D gbuf2 = sampler_state
{
	Address = Clamp;
};

sampler2D gbuf3 = sampler_state
{
	Address = Clamp;
};

samplerCube ambientMap = sampler_state
{
	Address = Clamp;
	Filter = Bilinear;
	MaxAnisotropy = 1;
};

// Contexts
context AMBIENT
{
	VertexShader = compile GLSL VS_FSQUAD;
	PixelShader = compile GLSL FS_AMBIENT;
	
	ZWriteEnable = false;
	BlendMode = Replace;
}

context LIGHTING
{
	VertexShader = compile GLSL VS_VOLUME;
	PixelShader = compile GLSL FS_LIGHTING;
	
	ZWriteEnable = false;
	BlendMode = Add;
}

context COPY_DEPTH
{
	VertexShader = compile GLSL VS_FSQUAD;
	PixelShader = compile GLSL FS_COPY_DEPTH;
}


[[VS_FSQUAD]]

float4x4 projMat;

struct VS_INPUT
{
	float3 vertPos	  : POSITION;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float2 texCoords : TEXCOORD0;
};
				
VS_OUTPUT main( VS_INPUT In )
{
	VS_OUTPUT Out = (VS_OUTPUT)0;

	Out.texCoords = In.vertPos.xy; 
	Out.position = mul( projMat, float4( In.vertPos, 1 ) );
	
	return Out;
}


[[VS_VOLUME]]

float4x4 viewProjMat;
float4x4 worldMat;

struct VS_INPUT
{
	float3 vertPos	  : POSITION;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float4 vpos : TEXCOORD0;
};
			
VS_OUTPUT main( VS_INPUT In )
{
	VS_OUTPUT Out = (VS_OUTPUT)0;
	Out.vpos = mul( viewProjMat, mul( worldMat, float4( In.vertPos, 1 ) ) );
	Out.position = Out.vpos;
	
	return Out;
}


[[FS_AMBIENT]]

#include "shaders/d3d11/utilityLib/fragDeferredRead.hlsl"

TextureCube texture_ambientMap;
SamplerState sampler_ambientMap;

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float2 texCoords : TEXCOORD0;
};

float3 main( VS_OUTPUT In ) : SV_Target
{
	if( getMatID( In.texCoords ) == 0.0 )	// Background
	{
		return float3( 0, 0, 0 );
	}
	else if( getMatID( In.texCoords ) == 2.0 )	// Sky
	{
		return getAlbedo( In.texCoords );
	}
	else
	{
		return getAlbedo( In.texCoords ) * texture_ambientMap.Sample( sampler_ambientMap, getNormal( In.texCoords ) ).rgb;
	}
}


[[FS_LIGHTING]]

#include "shaders/d3d11/utilityLib/fragLighting.hlsl"
#include "shaders/d3d11/utilityLib/fragDeferredRead.hlsl"

float4x4 viewMat;

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float4 vpos : TEXCOORD0;
};

float3 main( VS_OUTPUT In ) : SV_Target
{
	float2 fragCoord = (In.vpos.xy / In.vpos.w) * 0.5 + 0.5;
	
	if( getMatID( fragCoord ) == 1.0 )	// Standard phong material
	{
		float3 pos = getPos( fragCoord ) + viewerPos;
		float vsPos = mul(viewMat, float4( pos, 1.0 )).z;
		float4 specParams = getSpecParams( fragCoord );
		
		return calcPhongSpotLight( pos, getNormal( fragCoord ),
								getAlbedo( fragCoord ), specParams.rgb, specParams.a, -vsPos, 0.3 );
	}
	else discard;
	
	return float3(0.0, 0.0, 0.0);
}


[[FS_COPY_DEPTH]]

Texture2D texture_depthBuf;
SamplerState sampler_depthBuf;

float main( float2 texCoords : TEXCOORD0 ) : SV_Depth
{
	return texture_depthBuf.Sample( sampler_depthBuf, texCoords ).r;
}
