struct ParticleDataType
{
    float4 PositionWorld;
    float4 PositionImage : POSITION;
    float3 Velocity;
    float VelocityLength;
};

StructuredBuffer<ParticleDataType> Particles : register(t0);

struct VertexInput
{
 	uint VertexID : SV_VertexID;
};

struct PixelInput
{
    float4 Position : SV_POSITION;
    float4 Velocity : COLOR0;
};

PixelInput ParticleVS(VertexInput input)
{
    const uint mapIndex[6] = { 0, 1, 2, 0, 2, 3 };
    uint particleIndex = input.VertexID / 6;
    uint vertexIndex = input.VertexID % 6;
    uint index = particleIndex * 4 + mapIndex[vertexIndex];
    
    ParticleDataType data = Particles[index];

    PixelInput output;
    output.Position = data.PositionImage;
    output.Velocity.x = data.VelocityLength;
    
	return output;
}