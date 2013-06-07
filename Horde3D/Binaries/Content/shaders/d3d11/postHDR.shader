[[FX]]

// Samplers
sampler2D buf0 = sampler_state
{
	Address = Clamp;
};

sampler2D buf1 = sampler_state
{
	Address = Clamp;
};

// Uniforms
float hdrExposure = 2.0;       // Exposure (higher values make scene brighter)
float hdrBrightThres = 0.6;    // Brightpass threshold (intensity where blooming begins)
float hdrBrightOffset = 0.06;  // Brightpass offset (smaller values produce stronger blooming)

float4 blurParams = {0, 0, 0, 0};

// Contexts
context BRIGHTPASS
{
	VertexShader = compile GLSL VS_FSQUAD;
	PixelShader = compile GLSL FS_BRIGHTPASS;
	
	ZWriteEnable = false;
}

context BLUR
{
	VertexShader = compile GLSL VS_FSQUAD;
	PixelShader = compile GLSL FS_BLUR;
	
	ZWriteEnable = false;
}

context FINALPASS
{
	VertexShader = compile GLSL VS_FSQUAD;
	PixelShader = compile GLSL FS_FINALPASS;
	
	ZWriteEnable = false;
}


[[VS_FSQUAD]]
// =================================================================================================

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


[[FS_BRIGHTPASS]]
// =================================================================================================

#include "shaders/d3d11/utilityLib/fragPostProcess.hlsl"

Texture2D texture_buf0;
SamplerState sampler_buf0;

float2 frameBufSize;
//float hdrExposure;
float hdrBrightThres;
float hdrBrightOffset;

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float2 texCoords : TEXCOORD0;
};

float4 main( VS_OUTPUT In ) : SV_Target
{
	float2 texSize = frameBufSize * 4.0;
	float2 coord2 = In.texCoords + float2( 2, 2 ) / texSize;
	
	// Average using bilinear filtering
	float4 sum = getTex2DBilinear( texture_buf0, sampler_buf0, In.texCoords, texSize );
	sum += getTex2DBilinear( texture_buf0, sampler_buf0, coord2, texSize );
	sum += getTex2DBilinear( texture_buf0, sampler_buf0, float2( coord2.x, In.texCoords.y ), texSize );
	sum += getTex2DBilinear( texture_buf0, sampler_buf0, float2( In.texCoords.x, coord2.y ), texSize );
	sum /= 4.0;
	
	// Tonemap
	//sum = 1.0 - exp2( -hdrExposure * sum );
	
	// Extract bright values
	sum = max( sum - hdrBrightThres, 0.0 );
	sum /= hdrBrightOffset + sum;
	
	return sum;
}

	
[[FS_BLUR]]
// =================================================================================================

#include "shaders/d3d11/utilityLib/fragPostProcess.hlsl"

Texture2D texture_buf0;
SamplerState sampler_buf0;

float2 frameBufSize;
float4 blurParams;

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float2 texCoords : TEXCOORD0;
};

float4 main( VS_OUTPUT In ) : SV_Target
{
	return blurKawase( texture_buf0, sampler_buf0, In.texCoords, frameBufSize, blurParams.x );
}
	

[[FS_FINALPASS]]
// =================================================================================================

Texture2D	texture_buf0;
SamplerState sampler_buf0;
Texture2D	texture_buf1;
SamplerState sampler_buf1;

float2 frameBufSize;
float hdrExposure;

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float2 texCoords : TEXCOORD0;
};

float4 main( VS_OUTPUT In ) : SV_Target
{
	float4 col0 = texture_buf0.Sample( sampler_buf0, In.texCoords );	// HDR color
	float4 col1 = texture_buf1.Sample( sampler_buf1, In.texCoords );	// Bloom
	
	// Tonemap (using photographic exposure mapping)
	float4 col = 1.0 - exp2( -hdrExposure * col0 );
	
	return col + col1;
}
