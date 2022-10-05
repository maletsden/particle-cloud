cbuffer Params
{
	matrix ViewMatrix;
	matrix ProjectionMatrix;
};

struct VertexDataType
{
    float4 Position : POSITION;
    float2 UV : TEXCOORD0;
};

StructuredBuffer<VertexDataType> VertexShaderInput : register(t0);

struct VertexInput
{
 	uint VertexID : SV_VertexID;
};

struct PixelInput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
};

PixelInput ParticleVS(VertexInput input)
{
    VertexDataType data = VertexShaderInput[input.VertexID];

    PixelInput output;
    output.Position = mul(data.Position, ProjectionMatrix);
    output.UV = data.UV;
    
	return output;
}