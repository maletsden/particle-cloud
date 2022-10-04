struct Particle
{
    float3 Position;
    float3 Velocity;
};

//cbuffer Handler : register(b0)
//{
//    int GroupDim;
//    uint MaxParticles;
//    float DeltaTime;
//};

RWStructuredBuffer<Particle> Particles : register(u0);

//#define THREAD_GROUP_X 32
//#define THREAD_GROUP_Y 24
//#define THREAD_GROUP_TOTAL 768

//[numthreads(THREAD_GROUP_X, THREAD_GROUP_Y, 1)]
//void DefaultCS(uint3 groupID : SV_GroupID, uint groupIndex : SV_GroupIndex)
[numthreads(1, 1, 1)]
void DefaultCS(uint3 DTid : SV_DispatchThreadID)
{
    // uint index = groupID.x * THREAD_GROUP_TOTAL + groupID.y * GroupDim * THREAD_GROUP_TOTAL + groupIndex;
    uint index = DTid.x;
	
	[flatten]
    if (index >= 100000)
        return;

    Particle particle = Particles[index];

    float3 position = particle.Position;
    float3 velocity = particle.Velocity;

    particle.Position = position + velocity * 0.1f;
    particle.Velocity = velocity;

    Particles[index] = particle;
}

technique ParticleSolver
{
    pass DefaultPass
    {
        Profile = 11.0;
        ComputeShader = DefaultCS;
    }
}