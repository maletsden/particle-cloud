cbuffer Params
{
	matrix ViewMatrix;
	matrix ProjectionMatrix;
};

struct Particle
{
    float3 Position;
    float3 Velocity;
};

StructuredBuffer<Particle> Particles : register(t0);

struct VertexInput
{
 	uint VertexID : SV_VertexID;
};

struct PixelInput
{
	float4 Position : SV_POSITION; 
};

PixelInput ParticleVS(VertexInput input)
{
	PixelInput output = (PixelInput)0;

	Particle particle = Particles[input.VertexID];

	float4 worldPosition = float4(particle.Position, 1);
	float4 viewPosition = mul(worldPosition, ViewMatrix);
	output.Position = mul(viewPosition, ProjectionMatrix);

	return output;
}