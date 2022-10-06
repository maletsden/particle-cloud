struct Particle
{
    float3 Position;
    float3 Velocity;
};

struct VertexDataType
{
    float4 Position;
    float Velocity;
    float2 UV;
};

cbuffer Params : register(b0)
{
    matrix ViewMatrix;
    matrix ProjectionMatrix;
};

cbuffer GravityFieldBufferType : register(b1)
{
    float3 GPosition;
    float padding;
};

//cbuffer Handler : register(b1)
//{
//    int GroupDim;
//    uint MaxParticles;
//    float DeltaTime;
//};

RWStructuredBuffer<Particle> Particles : register(u0);
RWStructuredBuffer<VertexDataType> VertexShaderInput : register(u1);

float3 _calculateGravityForce(float3 particlePosition, float3 gravityFieldPosition)
{
    float3 direction = particlePosition - gravityFieldPosition;
    float distance = length(direction);
    return -direction / pow(distance, 3.0);
}

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
    if (index >= 1000000)
        return;
    
    Particle particle = Particles[index];
        
    // Move the particle.
    float3 position = particle.Position;
    float3 velocity = particle.Velocity;

    float deltaTime = 0.15f;
    float3 gravityAcceleration = _calculateGravityForce(position, GPosition);

    float3 halfNewVelocity = velocity + gravityAcceleration * (deltaTime / 2.0);
    float3 newPosition = position + halfNewVelocity * deltaTime;
    
    float3 newGravityAcceleration = _calculateGravityForce(newPosition, GPosition);
    float3 newVelocity = halfNewVelocity + newGravityAcceleration * (deltaTime / 2.0);
    
    particle.Position = newPosition;
    particle.Velocity = newVelocity;
    
    Particles[index] = particle;

    // Compute position of QuadBillboard.
    float4 worldPosition = float4(particle.Position, 1);
    float4 viewPosition = mul(worldPosition, ViewMatrix);
    float velocityLength = length(particle.Velocity);
    
    const float size = 0.01f;
    VertexShaderInput[index * 6 + 0].Position = viewPosition + float4(-1, -1, 0, 0) * size;
    VertexShaderInput[index * 6 + 0].Velocity = velocityLength;
    VertexShaderInput[index * 6 + 0].UV = float2(0, 1);
    
    VertexShaderInput[index * 6 + 1].Position = viewPosition + float4(-1, 1, 0, 0) * size;
    VertexShaderInput[index * 6 + 1].Velocity = velocityLength;
    VertexShaderInput[index * 6 + 1].UV = float2(0, 0);

    VertexShaderInput[index * 6 + 2].Position = viewPosition + float4(1, 1, 0, 0) * size;
    VertexShaderInput[index * 6 + 2].Velocity = velocityLength;
    VertexShaderInput[index * 6 + 2].UV = float2(1, 0);
    
    VertexShaderInput[index * 6 + 3].Position = viewPosition + float4(-1, -1, 0, 0) * size;
    VertexShaderInput[index * 6 + 3].Velocity = velocityLength;
    VertexShaderInput[index * 6 + 3].UV = float2(0, 1);
    
    
    VertexShaderInput[index * 6 + 4].Position = viewPosition + float4(1, 1, 0, 0) * size;
    VertexShaderInput[index * 6 + 4].Velocity = velocityLength;
    VertexShaderInput[index * 6 + 4].UV = float2(1, 0);
    
    VertexShaderInput[index * 6 + 5].Position = viewPosition + float4(1, -1, 0, 0) * size;
    VertexShaderInput[index * 6 + 5].Velocity = velocityLength;
    VertexShaderInput[index * 6 + 5].UV = float2(1, 1);
}

technique ParticleSolver
{
    pass DefaultPass
    {
        Profile = 11.0;
        ComputeShader = DefaultCS;
    }
}