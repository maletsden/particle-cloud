Texture2D shaderTexture;
SamplerState SampleType;

struct PixelInput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
    float2 Velocity : COLOR0;
};

struct PixelOutput
{
    float4 Color : SV_TARGET0;
};


float3 HueToRGB(float hue)
{
    float R = abs(hue * 6.0 - 3.0) - 1.0;
    float G = 2.0 - abs(hue * 6.0 - 2.0);
    float B = 2.0 - abs(hue * 6.0 - 4.0);
    return float3(R, G, B);
}

PixelOutput ParticlePS(PixelInput input)
{
	PixelOutput output = (PixelOutput)0;

    float maxVelocity = 2.0f;
    float velocity = min(input.Velocity.x, maxVelocity) / maxVelocity;
    float hue = (1.0 - velocity) * (4.0f / 6.0f);

    output.Color = float4(HueToRGB(hue), 0.5);
	
	return output;
}