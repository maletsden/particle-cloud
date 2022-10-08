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
    float2 velocityLength = float2(length(particle.Velocity), 0.0);
    float4 imagePosition = mul(viewPosition, ProjectionMatrix);
    
    const float size = 0.01f;
    float4 shift = mul(float4(size, size, 0, 0), ProjectionMatrix);
    
    // Bottom left.
    VertexShaderInput[index * 4 + 0].Position = imagePosition + float4(-shift.x, -shift.y, 0, 0);
    VertexShaderInput[index * 4 + 0].Velocity = velocityLength;
    VertexShaderInput[index * 4 + 0].UV = float2(0, 1);
    
    // Top left.
    VertexShaderInput[index * 4 + 1].Position = imagePosition + float4(-shift.x, shift.y, 0, 0);
    VertexShaderInput[index * 4 + 1].Velocity = velocityLength;
    VertexShaderInput[index * 4 + 1].UV = float2(0, 0);

    // Top Right.
    VertexShaderInput[index * 4 + 2].Position = imagePosition + float4(shift.x, shift.y, 0, 0);
    VertexShaderInput[index * 4 + 2].Velocity = velocityLength;
    VertexShaderInput[index * 4 + 2].UV = float2(1, 0);
    
    // Bottom right.
    VertexShaderInput[index * 4 + 3].Position = imagePosition + float4(shift.x, -shift.y, 0, 0);
    VertexShaderInput[index * 4 + 3].Velocity = velocityLength;
    VertexShaderInput[index * 4 + 3].UV = float2(1, 1);
}

technique ParticleSolver
{
    pass DefaultPass
    {
        Profile = 11.0;
        ComputeShader = DefaultCS;
    }
}