[[FX]]

// Supported Flags
/* ---------------
	_F01_Skinning
	_F02_NormalMapping
	_F03_ParallaxMapping
	_F04_EnvMapping
	_F05_AlphaTest
*/


// Samplers
sampler2D albedoMap = sampler_state
{
	Texture = "textures/common/white.tga";
};

sampler2D normalMap = sampler_state
{
	Texture = "textures/common/defnorm.tga";
};

samplerCube ambientMap = sampler_state
{
	Address = Clamp;
	Filter = Bilinear;
	MaxAnisotropy = 1;
};

samplerCube envMap = sampler_state
{
	Address = Clamp;
	Filter = Bilinear;
	MaxAnisotropy = 1;
};

// Uniforms
float4 matDiffuseCol <
	string desc_abc = "abc: diffuse color";
	string desc_d   = "d: alpha for opacity";
> = {1.0, 1.0, 1.0, 1.0};

float4 matSpecParams <
	string desc_abc = "abc: specular color";
	string desc_d   = "d: gloss";
> = {0.04, 0.04, 0.04, 0.5};

// Contexts
context ATTRIBPASS
{
	VertexShader = compile GLSL VS_GENERAL;
	PixelShader = compile GLSL FS_ATTRIBPASS;
}

context SHADOWMAP
{
	VertexShader = compile GLSL VS_SHADOWMAP;
	PixelShader = compile GLSL FS_SHADOWMAP;
}

context LIGHTING
{
	VertexShader = compile GLSL VS_GENERAL;
	PixelShader = compile GLSL FS_LIGHTING;
	
	ZWriteEnable = false;
	BlendMode = Add;
}

context AMBIENT
{
	VertexShader = compile GLSL VS_GENERAL;
	PixelShader = compile GLSL FS_AMBIENT;
}


[[VS_GENERAL]]
// =================================================================================================

#ifdef _F03_ParallaxMapping
	#define _F02_NormalMapping
#endif

#include "shaders/d3d11/utilityLib/vertCommon.hlsl"

#ifdef _F01_Skinning
#include "shaders/d3d11/utilityLib/vertSkinning.hlsl"
#endif

float4x4 viewProjMat;
float3 viewerPos;

struct VS_INPUT
{
	float3 vertPos	  : POSITION;
	float2 texCoords0 : TEXCOORD0;
	float3 normal	  : NORMAL;
#ifdef _F02_NormalMapping
	float4 tangent	  : TANGENT;
#endif
#ifdef _F01_Skinning
	float4 joints : BLENDINDICES;
	float4 weights : BLENDWEIGHTS;
#endif
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;

	float2 texCoords : TEXCOORD0;
	float4 pos : TEXCOORD1;
	float4 vsPos : TEXCOORD2;

#ifdef _F02_NormalMapping
	float3x3 tsbMat:TEXCOORD3;
#else
	float3 tsbNormal:TEXCOORD3;
#endif
#ifdef _F03_ParallaxMapping
	float3 eyeTS:TEXCOORD6;
#endif
};

VS_OUTPUT main( VS_INPUT In )
{
	VS_OUTPUT Out = (VS_OUTPUT)0;
#ifdef _F01_Skinning
	float4x4 skinningMat = calcSkinningMat(In.weights, In.joints);
	float3x3 skinningMatVec = getSkinningMatVec( skinningMat );
#endif
	
	// Calculate normal
#ifdef _F01_Skinning
	float3 _normal = normalize( calcWorldVec( skinVec( In.normal, skinningMatVec ) ) );
#else
	float3 _normal = normalize( calcWorldVec( In.normal ) );
#endif

	// Calculate tangent and bitangent
#ifdef _F02_NormalMapping
	#ifdef _F01_Skinning
		float3 _tangent = normalize( calcWorldVec( skinVec( In.tangent.xyz, skinningMatVec ) ) );
	#else
		float3 _tangent = normalize( calcWorldVec( In.tangent.xyz ) );
	#endif
	
	float3 _bitangent = cross( _normal, _tangent ) * In.tangent.w;
	Out.tsbMat = calcTanToWorldMat( _tangent, _bitangent, _normal );
#else
	Out.tsbNormal = _normal;
#endif

	// Calculate world space position
#ifdef _F01_Skinning	
	Out.pos = calcWorldPos( skinPos( float4( In.vertPos, 1.0 ), skinningMat ) );
#else
	Out.pos = calcWorldPos( float4( In.vertPos, 1.0 ) );
#endif

	Out.vsPos = calcViewPos( Out.pos );

	// Calculate tangent space eye vector
#ifdef _F03_ParallaxMapping
	Out.eyeTS = calcTanVec( viewerPos - Out.pos.xyz, _tangent, _bitangent, _normal );
#endif
	
	// Calculate texture coordinates and clip space position
	Out.texCoords = In.texCoords0;
	Out.position = mul(viewProjMat, Out.pos);

	return Out;
}


[[FS_ATTRIBPASS]]
// =================================================================================================

#ifdef _F03_ParallaxMapping
	#define _F02_NormalMapping
#endif

#include "shaders/d3d11/utilityLib/fragDeferredWrite.hlsl" 

float3 viewerPos;
float4 matDiffuseCol;
float4 matSpecParams;

Texture2D texture_albedoMap;
SamplerState sampler_albedoMap;

#ifdef _F02_NormalMapping
	SamplerState sampler_normalMap;
	Texture2D texture_normalMap;
#endif

struct VS_OUTPUT
{
	float4 position : SV_POSITION;

	float2 texCoords : TEXCOORD0;
	float4 pos : TEXCOORD1;
	float4 vsPos : TEXCOORD2;	//not used but d3d11 needs this for mapping

#ifdef _F02_NormalMapping
	float3x3 tsbMat:TEXCOORD3;
#else
	float3 tsbNormal:TEXCOORD3;
#endif
#ifdef _F03_ParallaxMapping
	float3 eyeTS:TEXCOORD6;
#endif
};

fragDefferedBuffer main( VS_OUTPUT In )
{
	fragDefferedBuffer Out = (fragDefferedBuffer)0;
	float3 newCoords = float3( In.texCoords, 0 );
	
#ifdef _F03_ParallaxMapping	
	const float plxScale = 0.03;
	const float plxBias = -0.015;
	
	// Iterative parallax mapping
	float3 eye = normalize( In.eyeTS );
	for( int i = 0; i < 4; ++i )
	{
		float4 nmap = texture_normalMap.Sample(sampler_normalMap, newCoords.xy * float2( 1, -1 ) );
		float height = nmap.a * plxScale + plxBias;
		newCoords += (height - newCoords.z) * nmap.z * eye;
	}
#endif

	// Flip texture vertically to match the GL coordinate system
	newCoords.y *= -1.0;

	float4 albedo = texture_albedoMap.Sample( sampler_albedoMap, newCoords.xy ) * matDiffuseCol;
	
#ifdef _F05_AlphaTest
	if( albedo.a < 0.01 ) discard;
#endif
	
#ifdef _F02_NormalMapping
	float3 normalMap = texture_normalMap.Sample( sampler_normalMap, newCoords.xy ).rgb * 2.0 - 1.0;
	float3 normal = mul( normalMap, In.tsbMat);
#else
	float3 normal = In.tsbNormal;
#endif

	float3 newPos = In.pos.xyz;

#ifdef _F03_ParallaxMapping
	newPos += float3( 0.0, newCoords.z, 0.0 );
#endif
	
	setMatID( Out, 1.0 );
	setPos( Out, newPos - viewerPos );
	setNormal( Out, normalize( normal ) );
	setAlbedo( Out, albedo.rgb );
	setSpecParams( Out, matSpecParams.rgb, matSpecParams.a );
	
	return Out;
}

	
[[VS_SHADOWMAP]]
// =================================================================================================
	
#include "shaders/d3d11/utilityLib/vertCommon.hlsl"
#include "shaders/d3d11/utilityLib/vertSkinning.hlsl"

float4x4 viewProjMat;
float4 lightPos;

struct VS_INPUT
{
	float3 vertPos	  : POSITION;
#ifdef _F05_AlphaTest
	float2 texCoords0 : TEXCOORD0;
#endif
#ifdef _F01_Skinning
	float4 joints : BLENDINDICES;
	float4 weights : BLENDWEIGHTS;
#endif
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;

#ifdef _F05_AlphaTest
	float2 texCoords : TEXCOORD0;
#endif
	float3 lightVec : TEXCOORD1;
};

VS_OUTPUT main( VS_INPUT In )
{
	VS_OUTPUT Out = (VS_OUTPUT)0;
#ifdef _F01_Skinning	
	float4 pos = calcWorldPos( skinPos( float4( In.vertPos, 1.0 ), In.weights, In.joints ) );
#else
	float4 pos = calcWorldPos( float4( In.vertPos, 1.0 ) );
#endif

#ifdef _F05_AlphaTest
	Out.texCoords = In.texCoords0;
#endif

	Out.lightVec = lightPos.xyz - pos.xyz;
	Out.position = mul(viewProjMat, pos);
	
	return Out;
}
	
	
[[FS_SHADOWMAP]]
// =================================================================================================

struct VS_OUTPUT
{
	float4 position : SV_POSITION;

#ifdef _F05_AlphaTest
	float2 texCoords : TEXCOORD0;
#endif
	float3 lightVec : TEXCOORD1;
};

float4 lightPos;
float shadowBias;

#ifdef _F05_AlphaTest
	float4 matDiffuseCol;
	Texture2D texture_albedoMap;
	SamplerState sampler_albedoMap;
#endif

float main( VS_OUTPUT In ) : SV_Depth
{
#ifdef _F05_AlphaTest
	float4 albedo = texture_albedoMap.Sample( sampler_albedoMap, texCoords * float2( 1, -1 ) ) * matDiffuseCol;
	if( albedo.a < 0.01 ) discard;
#endif
	
	float dist = length( In.lightVec ) / lightPos.w;
	float depth = dist + shadowBias;
	
	// Clearly better bias but requires SM 3.0
	//float depth = dist + abs( dFdx( dist ) ) + abs( dFdy( dist ) ) + shadowBias;
	
	return depth;
}


[[FS_LIGHTING]]
// =================================================================================================

#ifdef _F03_ParallaxMapping
	#define _F02_NormalMapping
#endif

#include "shaders/d3d11/utilityLib/fragLighting.hlsl" 

float4 matDiffuseCol;
float4 matSpecParams;

Texture2D texture_albedoMap;
SamplerState sampler_albedoMap;

#ifdef _F02_NormalMapping
Texture2D texture_normalMap;
SamplerState sampler_normalMap;
#endif

struct VS_OUTPUT
{
	float4 position : SV_POSITION;

	float2 texCoords : TEXCOORD0;
	float4 pos : TEXCOORD1;
	float4 vsPos : TEXCOORD2;

#ifdef _F02_NormalMapping
	float3x3 tsbMat:TEXCOORD3;
#else
	float3 tsbNormal:TEXCOORD3;
#endif
#ifdef _F03_ParallaxMapping
	float3 eyeTS:TEXCOORD6;
#endif
};

float4 main( VS_OUTPUT In ) : SV_Target
{
	float3 newCoords = float3( In.texCoords, 0 );
	
#ifdef _F03_ParallaxMapping	
	const float plxScale = 0.03;
	const float plxBias = -0.015;
	
	// Iterative parallax mapping
	float3 eye = normalize( In.eyeTS );
	for( int i = 0; i < 4; ++i )
	{
		float4 nmap = texture_normalMap.Sample( sampler_normalMap, newCoords.xy * float2( 1, -1 ) );
		float height = nmap.a * plxScale + plxBias;
		newCoords += (height - newCoords.z) * nmap.z * eye;
	}
#endif

	// Flip texture vertically to match the GL coordinate system
	newCoords.y *= -1.0;	//should we flip in d3d??

	float4 albedo = texture_albedoMap.Sample( sampler_albedoMap, newCoords.xy ) * matDiffuseCol;
	
#ifdef _F05_AlphaTest
	if( albedo.a < 0.01 ) discard;
#endif
	
#ifdef _F02_NormalMapping
	float3 normalMap = texture_normalMap.Sample( sampler_normalMap, newCoords.xy ).rgb * 2.0 - 1.0;
	float3 normal = mul( normalMap, In.tsbMat);
#else
	float3 normal = In.tsbNormal;
#endif

	float3 newPos = In.pos.xyz;

#ifdef _F03_ParallaxMapping
	newPos += float3( 0.0, newCoords.z, 0.0 );
#endif
	
	return float4( calcPhongSpotLight( newPos, normalize( normal ), albedo.rgb, matSpecParams.rgb,
		                  matSpecParams.a, -In.vsPos.z, 0.3 ), 1.0f);
}


[[FS_AMBIENT]]	
// =================================================================================================

#ifdef _F03_ParallaxMapping
	#define _F02_NormalMapping
#endif

#include "shaders/d3d11/utilityLib/fragLighting.hlsl" 

SamplerState sampler_albedoMap;
Texture2D texture_albedoMap;

SamplerState sampler_ambientMap;
TextureCube texture_ambientMap;

#ifdef _F02_NormalMapping
	SamplerState sampler_normalMap;
	Texture2D texture_normalMap;
#endif

#ifdef _F04_EnvMapping
	SamplerState sampler_envMap;
	TextureCube texture_envMap;
#endif

struct VS_OUTPUT
{
	float4 position : SV_POSITION;

	float2 texCoords : TEXCOORD0;
	float4 pos : TEXCOORD1;
	float4 vsPos : TEXCOORD2;

#ifdef _F02_NormalMapping
	float3x3 tsbMat:TEXCOORD3;
#else
	float3 tsbNormal:TEXCOORD3;
#endif
#ifdef _F03_ParallaxMapping
	float3 eyeTS:TEXCOORD6;
#endif
};

float4 main( VS_OUTPUT In ) : SV_Target
{
	float3 newCoords = float3( In.texCoords, 0 );
	
#ifdef _F03_ParallaxMapping	
	const float plxScale = 0.03;
	const float plxBias = -0.015;
	
	// Iterative parallax mapping
	float3 eye = normalize( In.eyeTS );
	for( int i = 0; i < 4; ++i )
	{
		float4 nmap = texture_normalMap.Sample( sampler_normalMap, newCoords.xy * float2( 1, -1 ) );
		float height = nmap.a * plxScale + plxBias;
		newCoords += (height - newCoords.z) * nmap.z * eye;
	}
#endif

	// Flip texture vertically to match the GL coordinate system
	newCoords.y *= -1.0;

	float4 albedo = texture_albedoMap.Sample( sampler_albedoMap, newCoords.xy );
	
#ifdef _F05_AlphaTest
	if( albedo.a < 0.01 ) discard;
#endif
	
#ifdef _F02_NormalMapping
	float3 normalMap = texture_normalMap.Sample( sampler_normalMap, newCoords.xy ).rgb * 2.0 - 1.0;
	float3 normal = mul(In.tsbMat, normalMap);
#else
	float3 normal = In.tsbNormal;
#endif

	float4 fragColor;
	fragColor.a = 1.0f;
		
	fragColor.rgb = albedo.rgb * texture_ambientMap.Sample( sampler_ambientMap, normal ).rgb;

#ifdef _F04_EnvMapping
	float3 refl = texture_envMap.Sample( sampler_envMap, reflect( In.pos.xyz - viewerPos, normalize( normal ) ) ).rgb;
	fragColor.rgb = refl * 1.5;
#endif

	return fragColor;
}
