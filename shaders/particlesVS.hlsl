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
    ParticleDataType data = Particles[input.VertexID];

    PixelInput output;
    output.Position = data.PositionImage;
    output.Velocity.x = data.VelocityLength;
    
	return output;
}