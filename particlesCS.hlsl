struct Particle
{
    float3 Position;
    float3 Velocity;
};

struct VertexDataType
{
    float4 Position;
    float2 UV;
};

cbuffer Params : register(b0)
{
    matrix ViewMatrix;
    matrix ProjectionMatrix;
};

//cbuffer Handler : register(b1)
//{
//    int GroupDim;
//    uint MaxParticles;
//    float DeltaTime;
//};

RWStructuredBuffer<Particle> Particles : register(u0);
RWStructuredBuffer<VertexDataType> VertexShaderInput : register(u1);

VertexDataType _offsetNprojected(float4 position, float2 offset, float2 uv)
{
    VertexDataType data;
    data.Position = position;
    data.Position.xy += offset;
    data.UV = uv;

    return data;
}

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
    
    // Move the particle.
    float3 position = particle.Position;
    float3 velocity = particle.Velocity;

    particle.Position = position + velocity * 0.01f;
    particle.Velocity = velocity;
    
    Particles[index] = particle;

    // Compute position of QuadBillboard.
    float4 worldPosition = float4(particle.Position, 1);
    float4 viewPosition = mul(worldPosition, ViewMatrix);
    
    const float size = 0.05f;
    VertexShaderInput[index * 6 + 0] = _offsetNprojected(viewPosition, float2(-1, -1) * size, float2(0, 1));
    VertexShaderInput[index * 6 + 1] = _offsetNprojected(viewPosition, float2(-1, 1) * size, float2(0, 0));
    VertexShaderInput[index * 6 + 2] = _offsetNprojected(viewPosition, float2(1, 1) * size, float2(1, 0));
    VertexShaderInput[index * 6 + 3] = _offsetNprojected(viewPosition, float2(-1, -1) * size, float2(0, 1));
    VertexShaderInput[index * 6 + 4] = _offsetNprojected(viewPosition, float2(1, 1) * size, float2(1, 0));
    VertexShaderInput[index * 6 + 5] = _offsetNprojected(viewPosition, float2(1, -1) * size, float2(1, 1));

}

technique ParticleSolver
{
    pass DefaultPass
    {
        Profile = 11.0;
        ComputeShader = DefaultCS;
    }
}