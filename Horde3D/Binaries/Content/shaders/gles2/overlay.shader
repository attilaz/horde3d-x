[[FX]]

// Samplers
sampler2D albedoMap;

// Contexts
context OVERLAY
{
	VertexShader = compile GLSL VS_OVERLAY;
	PixelShader = compile GLSL FS_OVERLAY;
	
	ZWriteEnable = false;
	BlendMode = Blend;
}

[[VS_OVERLAY]]

uniform mediump mat4 projMat;
attribute mediump vec2 vertPos;
attribute mediump vec2 texCoords0;
varying mediump vec2 texCoords;

void main( void )
{
	texCoords = vec2( texCoords0.s, -texCoords0.t ); 
	gl_Position = projMat * vec4( vertPos.x, vertPos.y, 1, 1 );
}


[[FS_OVERLAY]]

uniform mediump vec4 olayColor;
uniform mediump sampler2D albedoMap;
varying mediump vec2 texCoords;

void main( void )
{
	mediump vec4 albedo = texture2D( albedoMap, texCoords );
	
	gl_FragColor = albedo * olayColor;
}
