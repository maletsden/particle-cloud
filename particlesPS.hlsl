Texture2D shaderTexture;
SamplerState SampleType;

struct PixelInput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
};

struct PixelOutput
{
    float4 Color : SV_TARGET0;
};

PixelOutput ParticlePS(PixelInput input)
{
	PixelOutput output = (PixelOutput)0;

    output.Color = shaderTexture.Sample(SampleType, input.UV);
	
	return output;
}