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

matrix projMat;

struct VS_INPUT
{
	float2 vertPos : POSITION;
	float2 texCoords0 : TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float2 texCoords : TEXCOORD0;
};

VS_OUTPUT main( VS_INPUT In )
{
	VS_OUTPUT Out = (VS_OUTPUT)0;
	
	Out.texCoords = float2(In.texCoords0.x,1.0-In.texCoords0.y); 
	Out.position = mul( projMat, float4( In.vertPos.xy, 1.0, 1 ) );
	Out.position.z = Out.position.z * 0.5 + 0.5;
	
	return Out;
}

[[FS_OVERLAY]]

float4 olayColor;

Texture2D texture_albedoMap;
SamplerState sampler_albedoMap;

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float2 texCoords : TEXCOORD0;
};

float4 main( VS_OUTPUT In ) : SV_Target
{
	float4 albedo = texture_albedoMap.Sample(sampler_albedoMap, In.texCoords );
	
	return albedo; //float4(1.0,1.0,1.0,0.5); //albedo * olayColor;
}
