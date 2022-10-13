struct ParticleDataType
{
    float4 PositionWorld;
    float4 PositionImage;
    float3 Velocity;
    float VelocityLength;
};

cbuffer Parameters : register(b0)
{
    Matrix ViewMatrix;
    Matrix ProjectionMatrix;
    float3 GravityFieldPosition;
    float DeltaTime;
};

RWStructuredBuffer<ParticleDataType> Particles : register(u0);

float3 _calculateGravityForce(float3 particlePosition, float3 gravityFieldPosition)
{
    float3 direction = particlePosition - gravityFieldPosition;
    float distance = length(direction);
    return -direction / pow(distance, 3.0f);
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
    
    ParticleDataType particle = Particles[index * 4 + 0];
        
    // Move the particle.
    float3 positionWorld = particle.PositionWorld.xyz;
    float3 velocity = particle.Velocity;

    float3 gravityAcceleration = _calculateGravityForce(positionWorld, GravityFieldPosition);

    float3 halfNewVelocity = velocity + gravityAcceleration * (DeltaTime / 2.0f);
    float3 newPositionWorld = positionWorld + halfNewVelocity * DeltaTime;
    
    float3 newGravityAcceleration = _calculateGravityForce(newPositionWorld, GravityFieldPosition);
    float3 newVelocity = halfNewVelocity + newGravityAcceleration * (DeltaTime / 2.0f);

    Particles[index * 4 + 0].PositionWorld = float4(newPositionWorld, 1.f);
    Particles[index * 4 + 0].Velocity = newVelocity;

    // Compute position of QuadBillboard.
    float4 worldPosition = float4(newPositionWorld, 1.f);
    float4 viewPosition = mul(worldPosition, ViewMatrix);
    float2 velocityLength = float2(length(particle.Velocity), 0.0f);
    float4 imagePosition = mul(viewPosition, ProjectionMatrix);
    
    const float size = 0.01f;
    float4 shift = mul(float4(size, size, 0.f, 0.f), ProjectionMatrix);
    
    // Bottom left.
    Particles[index * 4 + 0].PositionImage = imagePosition + float4(-shift.x, -shift.y, 0, 0);
    Particles[index * 4 + 0].VelocityLength = velocityLength;
    
    // Top left.
    Particles[index * 4 + 1].PositionImage = imagePosition + float4(-shift.x, shift.y, 0, 0);
    Particles[index * 4 + 1].VelocityLength = velocityLength;

    // Top Right.
    Particles[index * 4 + 2].PositionImage = imagePosition + float4(shift.x, shift.y, 0, 0);
    Particles[index * 4 + 2].VelocityLength = velocityLength;
    
    // Bottom right.
    Particles[index * 4 + 3].PositionImage = imagePosition + float4(shift.x, -shift.y, 0, 0);
    Particles[index * 4 + 3].VelocityLength = velocityLength;
}

technique ParticleSolver
{
    pass DefaultPass
    {
        Profile = 11.0;
        ComputeShader = DefaultCS;
    }
}