// *************************************************************************************************
// Horde3D Shader Utility Library
// --------------------------------------
//		- Lighting functions -
//
// Copyright (C) 2006-2011 Nicolas Schulz
//	D3D11 version 2012-2013 Attila Kocsis
//
// You may use the following code in projects based on the Horde3D graphics engine.
//
// *************************************************************************************************

float3 viewerPos;
float4 lightPos;
float4 lightDir;
float3 lightColor;
Texture2D texture_shadowMap;
SamplerComparisonState sampler_shadowMap;
float4 shadowSplitDists;
float4x4 shadowMats[4];
float shadowMapSize;


float PCF( const float4 projShadow )
{
	// 5-tap PCF with a 30° rotated grid
	float offset = 1.0 / shadowMapSize;
	
	float shadow = texture_shadowMap.SampleCmpLevelZero( sampler_shadowMap, projShadow.xy, projShadow.z );
	shadow += texture_shadowMap.SampleCmpLevelZero( sampler_shadowMap, projShadow.xy + float2( -0.866 * offset,  0.5 * offset ), projShadow.z );
	shadow += texture_shadowMap.SampleCmpLevelZero( sampler_shadowMap, projShadow.xy + float2( -0.866 * offset, -0.5 * offset ), projShadow.z );
	shadow += texture_shadowMap.SampleCmpLevelZero( sampler_shadowMap, projShadow.xy + float2(  0.866 * offset, -0.5 * offset ), projShadow.z );
	shadow += texture_shadowMap.SampleCmpLevelZero( sampler_shadowMap, projShadow.xy + float2(  0.866 * offset,  0.5 * offset ), projShadow.z );
	
	return shadow / 5.0;
}


float3 calcPhongSpotLight( const float3 pos, const float3 normal, const float3 albedo, const float3 specColor,
						 const float gloss, const float viewDist, const float ambientIntensity )
{
	float3 light = lightPos.xyz - pos;
	float lightLen = length( light );
	light /= lightLen;
	
	// Distance attenuation
	float lightDepth = lightLen / lightPos.w;
	float atten = max( 1.0 - lightDepth * lightDepth, 0.0 );
	
	// Spotlight falloff
	float angle = dot( lightDir.xyz, -light );
	atten *= clamp( (angle - lightDir.w) / 0.2, 0.0, 1.0 );
		
	// Lambert diffuse
	float NdotL = max( dot( normal, light ), 0.0 );
	atten *= NdotL;
		
	// Blinn-Phong specular with energy conservation
	float3 view = normalize( viewerPos - pos );
	float3 halfVec = normalize( light + view );
	float specExp = exp2( 10.0 * gloss + 1.0 );
	float3 specular = specColor * pow( max( dot( halfVec, normal ), 0.0 ), specExp );
	specular *= (specExp * 0.125 + 0.25);  // Normalization factor (n+2)/8
	
	// Shadows
	float shadowTerm = 1.0;
	if( atten * (shadowMapSize - 4.0) > 0.0 )  // Skip shadow mapping if default shadow map (size==4) is bound
	{
		float4 projShadow = mul( shadowMats[3], float4( pos, 1.0 ));
		if( viewDist < shadowSplitDists.x ) projShadow = mul( shadowMats[0], float4( pos, 1.0 ) );
		else if( viewDist < shadowSplitDists.y ) projShadow = mul( shadowMats[1], float4( pos, 1.0 ) );
		else if( viewDist < shadowSplitDists.z ) projShadow = mul( shadowMats[2], float4( pos, 1.0 ) );
		
		projShadow.z = lightDepth;
		projShadow.xy /= projShadow.w;
		
		shadowTerm = max( PCF( projShadow ), ambientIntensity ); 
	}
	
	// Final color
	return (albedo + specular) * lightColor * atten * shadowTerm;
}
