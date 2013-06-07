// *************************************************************************************************
// Horde3D Shader Utility Library
// --------------------------------------
//		- Postprocessing functions -
//
// Copyright (C) 2006-2011 Nicolas Schulz
//	D3D11 version 2012-2013 Attila Kocsis
//
// You may use the following code in projects based on the Horde3D graphics engine.
//
// *************************************************************************************************

float4 getTex2DBilinear( Texture2D tex, SamplerState texSampler, const float2 texCoord, const float2 texSize )
{
	// Bilinear filtering function. Useful when hardware filtering is not available, e.g. for
	// floating point textures on ATI 1xx0 cards
	
	float2 coord0 = texCoord - 0.5 / texSize;
	float2 coord1 = texCoord + 0.5 / texSize;
	float2 weight = frac( coord0 * texSize );
	
	float4 bot = lerp( tex.Sample( texSampler, coord0 ),
					tex.Sample( texSampler, float2( coord1.x, coord0.y ) ),
					weight.x );
	float4 top = lerp( tex.Sample( texSampler, float2( coord0.x, coord1.y ) ),
					tex.Sample( texSampler, coord1 ),
					weight.x );
	
	return lerp( bot, top, weight.y );
}

float4 blurKawase( Texture2D tex, SamplerState texSampler,  const float2 texCoord, const float2 texSize, const float iteration )
{
	// Function assumes that tex is using bilinear hardware filtering
	
	float2 dUV = (iteration + 0.5) / texSize;
	
	float4 col = tex.Sample( texSampler, texCoord + float2( -dUV.x, dUV.y ) );	// Top left
	col += tex.Sample( texSampler, texCoord + dUV );						// Top right
	col += tex.Sample( texSampler, texCoord + float2( dUV.x, -dUV.y ) );		// Bottom right
	col += tex.Sample( texSampler, texCoord - dUV );						// Bottom left
	
	return col * 0.25;
}