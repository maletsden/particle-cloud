struct Particle
{
    float3 Position;
    float3 Velocity;
    float2 Padding;
};

struct VertexDataType
{
    float4 Position;
    float2 UV;
    float2 Velocity;
};

cbuffer Parameters : register(b0)
{
    Matrix ViewMatrix;
    Matrix ProjectionMatrix;
    float3 GravityFieldPosition;
    float DeltaTime;
};


RWStructuredBuffer<Particle> Particles : register(u0);
RWStructuredBuffer<VertexDataType> VertexShaderInput : register(u1);

float3 _calculateGravityForce(float3 particlePosition, float3 gravityFieldPosition)
{
    float3 direction = particlePosition - gravityFieldPosition;
    float distance = length(direction);
    return -direction / pow(distance, 3.0f);
}

VertexDataType _offsetNprojected(float4 position, float2 offset, float2 uv)
{
    VertexDataType data;
    data.Position = position;
    data.Position.xy += offset;
    data.UV = uv;

    return data;
}

#define THREAD_GROUP_X 32
#define THREAD_GROUP_Y 32
#define THREAD_GROUP_TOTAL 1024

[numthreads(THREAD_GROUP_X, THREAD_GROUP_Y, 1)]
void DefaultCS(uint3 groupID : SV_GroupID, uint groupIndex : SV_GroupIndex)
{    
    uint index = groupID.x * THREAD_GROUP_TOTAL + groupID.y * THREAD_GROUP_X * THREAD_GROUP_TOTAL + groupIndex;
	
	[flatten]
    if (index >= 1000000)
        return;
    
    Particle particle = Particles[index];
        
    // Move the particle.
    float3 position = particle.Position;
    float3 velocity = particle.Velocity;

    float3 gravityAcceleration = _calculateGravityForce(position, GravityFieldPosition);

    float3 halfNewVelocity = velocity + gravityAcceleration * (DeltaTime / 2.0f);
    float3 newPosition = position + halfNewVelocity * DeltaTime;
    
    float3 newGravityAcceleration = _calculateGravityForce(newPosition, GravityFieldPosition);
    float3 newVelocity = halfNewVelocity + newGravityAcceleration * (DeltaTime / 2.0f);
    
    particle.Position = newPosition;
    particle.Velocity = newVelocity;
    
    Particles[index] = particle;

    // Compute position of QuadBillboard.
    float4 worldPosition = float4(particle.Position, 1.f);
    float4 viewPosition = mul(worldPosition, ViewMatrix);
    float2 velocityLength = float2(length(particle.Velocity), 0.0f);
    float4 imagePosition = mul(viewPosition, ProjectionMatrix);
    
    const float size = 0.01f;
    float4 shift = mul(float4(size, size, 0.f, 0.f), ProjectionMatrix);
    
    // Bottom left.
    VertexShaderInput[index * 4 + 0].Position = imagePosition + float4(-shift.x, -shift.y, 0, 0);
    VertexShaderInput[index * 4 + 0].Velocity = velocityLength;
    VertexShaderInput[index * 4 + 0].UV = float2(0.f, 1.f);
    
    // Top left.
    VertexShaderInput[index * 4 + 1].Position = imagePosition + float4(-shift.x, shift.y, 0, 0);
    VertexShaderInput[index * 4 + 1].Velocity = velocityLength;
    VertexShaderInput[index * 4 + 1].UV = float2(0.f, 0.f);

    // Top Right.
    VertexShaderInput[index * 4 + 2].Position = imagePosition + float4(shift.x, shift.y, 0, 0);
    VertexShaderInput[index * 4 + 2].Velocity = velocityLength;
    VertexShaderInput[index * 4 + 2].UV = float2(1.f, 0.f);
    
    // Bottom right.
    VertexShaderInput[index * 4 + 3].Position = imagePosition + float4(shift.x, -shift.y, 0, 0);
    VertexShaderInput[index * 4 + 3].Velocity = velocityLength;
    VertexShaderInput[index * 4 + 3].UV = float2(1.f, 1.f);
}

technique ParticleSolver
{
    pass DefaultPass
    {
        Profile = 11.0;
        ComputeShader = DefaultCS;
    }
}