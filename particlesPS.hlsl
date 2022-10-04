struct PixelInput
{
	float4 Position : SV_POSITION; 
};

struct PixelOutput
{
    float4 Color : SV_TARGET0;
};

PixelOutput ParticlePS(PixelInput input)
{
	PixelOutput output = (PixelOutput)0;

	output.Color = float4((float3)1.0f, 1.0f);
	
	return output;
}