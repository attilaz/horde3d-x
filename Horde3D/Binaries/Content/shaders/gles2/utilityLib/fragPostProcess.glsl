// *************************************************************************************************
// Horde3D Shader Utility Library
// --------------------------------------
//		- Postprocessing functions -
//
// Copyright (C) 2006-2011 Nicolas Schulz
//
// You may use the following code in projects based on the Horde3D graphics engine.
//
// *************************************************************************************************

mediump vec4 getTex2DBilinear( const sampler2D tex, const mediump vec2 texCoord, const lowp vec2 texSize )
{
	// Bilinear filtering function. Useful when hardware filtering is not available, e.g. for
	// floating point textures on ATI 1xx0 cards
	
	mediump vec2 coord0 = texCoord - 0.5 / texSize;
	mediump vec2 coord1 = texCoord + 0.5 / texSize;
	lowp vec2 weight = fract( coord0 * texSize );
	
	mediump vec4 bot = mix( texture2D( tex, coord0 ),
					texture2D( tex, vec2( coord1.x, coord0.y ) ),
					weight.x );
	mediump vec4 top = mix( texture2D( tex, vec2( coord0.x, coord1.y ) ),
					texture2D( tex, coord1 ),
					weight.x );
	
	return mix( bot, top, weight.y );
}

mediump vec4 blurKawase( const sampler2D tex, const mediump vec2 texCoord, const lowp vec2 texSize, const lowp float iteration )
{
	// Function assumes that tex is using bilinear hardware filtering
	
	lowp vec2 dUV = (iteration + 0.5) / texSize;
	
	mediump vec4 col = texture2D( tex, texCoord + vec2( -dUV.x, dUV.y ) );	// Top left
	col += texture2D( tex, texCoord + dUV );						// Top right
	col += texture2D( tex, texCoord + vec2( dUV.x, -dUV.y ) );		// Bottom right
	col += texture2D( tex, texCoord - dUV );						// Bottom left
	
	return col * 0.25;
}