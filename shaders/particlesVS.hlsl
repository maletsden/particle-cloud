struct VertexDataType
{
    float4 Position : POSITION;
    float2 UV : TEXCOORD0;
    float2 Velocity;
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
    float2 Velocity : COLOR0;
};

PixelInput ParticleVS(VertexInput input)
{
    const uint mapIndex[6] = { 0, 1, 2, 0, 2, 3 };
    uint particleIndex = input.VertexID / 6;
    uint vertexIndex = input.VertexID % 6;
    uint index = particleIndex * 4 + mapIndex[vertexIndex];
    
    VertexDataType data = VertexShaderInput[index];

    PixelInput output;
    output.Position = data.Position;
    output.Velocity = data.Velocity;
    output.UV = data.UV;
    
	return output;
}