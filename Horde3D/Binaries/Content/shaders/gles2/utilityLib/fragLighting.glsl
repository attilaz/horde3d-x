// *************************************************************************************************
// Horde3D Shader Utility Library
// --------------------------------------
//		- Lighting functions -
//
// Copyright (C) 2006-2011 Nicolas Schulz
//
// You may use the following code in projects based on the Horde3D graphics engine.
//
// *************************************************************************************************
 #extension GL_EXT_shadow_samplers : enable

uniform mediump	vec3 viewerPos;
uniform mediump	vec4 lightPos;
uniform mediump	vec4 lightDir;
uniform mediump	vec3 lightColor;
#ifdef GL_EXT_shadow_samplers
uniform sampler2DShadow shadowMap;
#endif
uniform mediump	vec4 shadowSplitDists;
uniform mediump	mat4 shadowMats[4];
uniform mediump	float shadowMapSize;

mediump float unpack( mediump vec4 packedZValue)
{	
	const mediump vec4 unpackFactors = vec4( 1.0 / (16.0 * 16.0 * 16.0), 1.0 / (16.0 * 16.0), 1.0 / 16.0, 1.0 );
	return dot(packedZValue,unpackFactors);
}


mediump float PCF( const mediump vec4 projShadow )
{
	// 5-tap PCF with a 30 rotated grid
	
	mediump float offset = 1.0 / shadowMapSize;
	
#ifdef GL_EXT_shadow_samplers	
	mediump float shadow = shadow2DEXT( shadowMap, projShadow.xyz );
	shadow += shadow2DEXT( shadowMap, projShadow.xyz + mediump vec3( -0.866 * offset,  0.5 * offset, 0.0 ) );
	shadow += shadow2DEXT( shadowMap, projShadow.xyz + mediump vec3( -0.866 * offset, -0.5 * offset, 0.0 ) );
	shadow += shadow2DEXT( shadowMap, projShadow.xyz + mediump vec3(  0.866 * offset, -0.5 * offset, 0.0 ) );
	shadow += shadow2DEXT( shadowMap, projShadow.xyz + mediump vec3(  0.866 * offset,  0.5 * offset, 0.0 ) );
	
	return shadow / 5.0;
#else	
	return 1.0;
#endif	
}

mediump vec3 calcPhongSpotLight( const mediump vec3 pos, const mediump vec3 normal, const mediump vec3 albedo, const mediump vec3 specColor,
						 const mediump float gloss, const mediump float viewDist, const mediump float ambientIntensity )
{
	mediump vec3 light = lightPos.xyz - pos;
	mediump float lightLen = length( light );
	light /= lightLen;
	
	// Distance attenuation
	mediump float lightDepth = lightLen / lightPos.w;
	mediump float atten = max( 1.0 - lightDepth * lightDepth, 0.0 );
	
	// Spotlight falloff
	mediump float angle = dot( lightDir.xyz, -light );
	atten *= clamp( (angle - lightDir.w) / 0.2, 0.0, 1.0 );
		
	// Lambert diffuse contribution
	mediump float NdotL = max( dot( normal, light ), 0.0 );
	atten *= NdotL;	
	

	// Blinn-Phong specular with energy conservation
	mediump vec3 view = normalize( viewerPos - pos );
	mediump vec3 halfVec = normalize( light + view );
	mediump float specExp = exp2( 10.0 * gloss + 1.0 );
	mediump vec3 specular = specColor * pow( max( dot( halfVec, normal ), 0.0 ), specExp );
	specular *= (specExp * 0.125 + 0.25);  // Normalization factor (n+2)/8	
	
	// Shadow
	mediump float shadowTerm = 1.0;
    /*
	if( atten * (shadowMapSize - 4.0) > 0.0 )  // Skip shadow mapping if default shadow map (size==4) is bound
	{
		mediump vec4 projShadow = shadowMats[3] * mediump vec4( pos, 1.0 );
		if( viewDist < shadowSplitDists.x ) projShadow = shadowMats[0] * mediump vec4( pos, 1.0 );
		else if( viewDist < shadowSplitDists.y ) projShadow = shadowMats[1] * mediump vec4( pos, 1.0 );
		else if( viewDist < shadowSplitDists.z ) projShadow = shadowMats[2] * mediump vec4( pos, 1.0 );
		
		projShadow.z = lightDepth;
		projShadow.xy /= projShadow.w;
		
		shadowTerm = max( PCF( projShadow ), ambientIntensity );
	}
	*/
	// Final color
	return (albedo + specular) * lightColor * atten * shadowTerm;
}

