#ifndef _LIGHTSHADERCLASS_H_
#define _LIGHTSHADERCLASS_H_

#include <string_view>
#include <vector>
#include <memory>

#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxtk/SimpleMath.h>

using namespace DirectX::SimpleMath;

class ParticlesShader
{
private:
    struct MatrixBufferType
    {
        Matrix view;
        Matrix projection;
    };

    struct ParticleDataType
    {
        Vector3 position;
        Vector3 velocity;
    };

public:
    ParticlesShader();
    ~ParticlesShader();

    bool Initialize(ID3D11Device* device, HWND hwnd);
    void Shutdown();
    bool Render(ID3D11DeviceContext* deviceContext, int indexCount, Matrix viewMatrix, Matrix projectionMatrix);

private:
    bool InitializeShader(
        ID3D11Device* device,
        HWND hwnd,
        std::wstring_view vsFilename,
        std::wstring_view psFilename,
        std::wstring_view csFilename);
    void ShutdownShader();
    void OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, std::wstring_view shaderFilename);

    bool SetShaderParameters(ID3D11DeviceContext* deviceContext, Matrix viewMatrix, Matrix projectionMatrix);
    void RenderShader(ID3D11DeviceContext* deviceContext, int indexCount);

    bool InitializeComputeShader(ID3D11Device* device, HWND hwnd, std::wstring_view filename);

    void RunComputeShader(ID3D11DeviceContext* deviceContext);

private:
    constexpr static size_t s_ParticlesNumber = 100'000;

    ID3D11VertexShader* m_vertexShader;
    ID3D11PixelShader* m_pixelShader;
    ID3D11ComputeShader* m_computeShader;
    ID3D11InputLayout* m_layout;
    ID3D11SamplerState* m_sampleState;
    ID3D11Buffer* m_matrixBuffer;
    ID3D11Buffer* m_particlesBuffer;
    ID3D11ShaderResourceView* m_particlesSRV;   
    ID3D11UnorderedAccessView* m_particlesUAV;  

    std::vector<ParticleDataType> m_particlesDataBuffer;
};

#endif
