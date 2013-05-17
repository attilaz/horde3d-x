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

uniform mediump mat4 projMat;
attribute mediump vec3 vertPos;
varying mediump vec2 texCoords;
				
void main( void )
{
	texCoords = vertPos.xy; 
	gl_Position = projMat * vec4( vertPos, 1 );
}


[[FS_BRIGHTPASS]]
// =================================================================================================

#include "shaders/gles2/utilityLib/fragPostProcess.glsl"

uniform sampler2D buf0;
uniform lowp vec2 frameBufSize;
//uniform float hdrExposure;
uniform lowp float hdrBrightThres;
uniform lowp float hdrBrightOffset;
varying mediump vec2 texCoords;

void main( void )
{
	lowp vec2 texSize = frameBufSize * 4.0;
	mediump vec2 coord2 = texCoords + vec2( 2, 2 ) / texSize;
	
	// Average using bilinear filtering
	mediump vec4 sum = getTex2DBilinear( buf0, texCoords, texSize );
	sum += getTex2DBilinear( buf0, coord2, texSize );
	sum += getTex2DBilinear( buf0, vec2( coord2.x, texCoords.y ), texSize );
	sum += getTex2DBilinear( buf0, vec2( texCoords.x, coord2.y ), texSize );
	sum /= 4.0;
	
	// Tonemap
	//sum = 1.0 - exp2( -hdrExposure * sum );
	
	// Extract bright values
	sum = max( sum - hdrBrightThres, 0.0 );
	sum /= hdrBrightOffset + sum;
	
	gl_FragColor = sum;
}

	
[[FS_BLUR]]
// =================================================================================================

#include "shaders/gles2/utilityLib/fragPostProcess.glsl"

uniform sampler2D buf0;
uniform lowp vec2 frameBufSize;
uniform lowp vec4 blurParams;
varying mediump vec2 texCoords;

void main( void )
{
	gl_FragColor = blurKawase( buf0, texCoords, frameBufSize, blurParams.x );
}
	

[[FS_FINALPASS]]
// =================================================================================================

uniform mediump sampler2D buf0, buf1;
uniform mediump vec2 frameBufSize;
uniform mediump float hdrExposure;
varying mediump vec2 texCoords;

void main( void )
{
	mediump vec4 col0 = texture2D( buf0, texCoords );	// HDR color
	mediump vec4 col1 = texture2D( buf1, texCoords );	// Bloom
	
	// Tonemap (using photographic exposure mapping)
	mediump vec4 col = 1.0 - exp2( -hdrExposure * col0 );
	
	gl_FragColor = col + col1;
}
