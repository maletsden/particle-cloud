#ifndef _LIGHTSHADERCLASS_H_
#define _LIGHTSHADERCLASS_H_

#include <chrono>
#include <memory>
#include <string_view>
#include <vector>

#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxtk/SimpleMath.h>

#include "TextureClass.h"

using namespace DirectX::SimpleMath;

class ParticlesShader
{
private:
    struct CSParametersBufferType
    {
        Matrix View;
        Matrix Projection;
        Vector3 GravityFieldPosition;
        float DeltaTime;
    };

    struct ParticleDataType
    {
        Vector4 PositionWorld;
        Vector4 PositionImage;
        Vector3 Velocity;
        float VelocityLength;
    };

public:
    ParticlesShader();
    ~ParticlesShader();

    bool Initialize(ID3D11Device* device, HWND hwnd, const int screenWidth, const int screenHeight);
    void Shutdown();
    bool Render(ID3D11DeviceContext* deviceContext, int indexCount, const Matrix& viewMatrix, const Matrix& projectionMatrix);
    void SetMousePosition(const Vector2& mousePosition) noexcept;

private:
    static std::vector<unsigned long> GenerateIndexBuffer(const unsigned long number) noexcept;

    bool InitializeShader(
        ID3D11Device* device,
        HWND hwnd,
        std::wstring_view vsFilename,
        std::wstring_view psFilename,
        std::wstring_view csFilename);

    bool InitializeTexture(ID3D11Device* device, std::wstring_view textureFilename);

    void ShutdownShader();
    void OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, std::wstring_view shaderFilename);

    bool SetShaderParameters(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* texture);
    void RenderShader(ID3D11DeviceContext* deviceContext, int indexCount);

    bool InitializeComputeShader(ID3D11Device* device, HWND hwnd, std::wstring_view filename);

    void RunComputeShader(ID3D11DeviceContext* deviceContext);

    bool UpdateGravityFieldPosition(const Matrix& viewMatrix, const Matrix& projectionMatrix);
    bool UpdateFrameDeltaTime() noexcept;
    bool UpdateTransformationMatrices(const Matrix& viewMatrix, const Matrix& projectionMatrix) noexcept;

private:
    constexpr static size_t s_ParticlesNumber = 1000000;
    constexpr static size_t s_VerticesNumber = s_ParticlesNumber * 4;
    constexpr static size_t s_VertixIndeciesNumber = s_ParticlesNumber * 6;

    ID3D11VertexShader* m_vertexShader;
    ID3D11PixelShader* m_pixelShader;
    ID3D11ComputeShader* m_computeShader;

    ID3D11Buffer* m_csParametersBuffer;
    ID3D11Buffer* m_particlesBuffer;
    ID3D11Buffer* m_indexBuffer;

    ID3D11UnorderedAccessView* m_particlesUAV;
    ID3D11ShaderResourceView* m_particlesSRV;

    std::vector<ParticleDataType> m_particlesDataBuffer;
    std::vector<unsigned long> m_indexDataBuffer;

    ID3D11SamplerState* m_sampleState;
    std::unique_ptr<TextureClass> m_Texture;
    Vector2 m_MousePosition;
    int m_ScreenWidth;
    int m_ScreenHeight;
    CSParametersBufferType m_CSParameters;
    std::chrono::high_resolution_clock::time_point m_lastSampleTime;
};

#endif
