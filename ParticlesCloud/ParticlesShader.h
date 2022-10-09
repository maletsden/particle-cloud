#ifndef _LIGHTSHADERCLASS_H_
#define _LIGHTSHADERCLASS_H_

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
    struct MatrixBufferType
    {
        Matrix view;
        Matrix projection;
    };

    struct GravityFieldBufferType
    {
        Vector3 position;
        float padding;
    };

    struct ParticleDataType
    {
        Vector3 position;
        Vector3 velocity;
        float padding[2];
    };

    struct VertexDataType
    {
        Vector4 position;
        Vector2 uv;
        Vector2 velocity;
    };

public:
    ParticlesShader();
    ~ParticlesShader();

    bool Initialize(ID3D11Device* device, HWND hwnd, const int screenWidth, const int screenHeight);
    void Shutdown();
    bool Render(ID3D11DeviceContext* deviceContext, int indexCount, Matrix viewMatrix, Matrix projectionMatrix);
    void SetMousePosition(const Vector2& mousePosition) noexcept;

private:
    bool InitializeShader(
        ID3D11Device* device,
        HWND hwnd,
        std::wstring_view vsFilename,
        std::wstring_view psFilename,
        std::wstring_view csFilename);

    bool InitializeTexture(ID3D11Device* device, std::wstring_view textureFilename);

    void ShutdownShader();
    void OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, std::wstring_view shaderFilename);

    bool SetShaderParameters(
        ID3D11DeviceContext* deviceContext,
        Matrix viewMatrix,
        Matrix projectionMatrix,
        ID3D11ShaderResourceView* texture);
    void RenderShader(ID3D11DeviceContext* deviceContext, int indexCount);

    bool InitializeComputeShader(ID3D11Device* device, HWND hwnd, std::wstring_view filename);

    void RunComputeShader(ID3D11DeviceContext* deviceContext);
    Vector3 getRayFromScreenSpace(const Vector2& position, Matrix viewMatrix, Matrix projectionMatrix);

    bool UpdateGravityFieldPosition(ID3D11DeviceContext* deviceContext, Matrix viewMatrix, Matrix projectionMatrix);

private:
    constexpr static size_t s_ParticlesNumber = 1000000;
    constexpr static size_t s_VerticesNumber = s_ParticlesNumber * 4;
    constexpr static size_t s_VertixIndeciesNumber = s_ParticlesNumber * 6;

    ID3D11VertexShader* m_vertexShader;
    ID3D11PixelShader* m_pixelShader;
    ID3D11ComputeShader* m_computeShader;

    ID3D11Buffer* m_matrixBuffer;
    ID3D11Buffer* m_gravityFieldBuffer;
    ID3D11Buffer* m_particlesBuffer;
    ID3D11Buffer* m_quadBillboardBuffer;

    ID3D11UnorderedAccessView* m_particlesUAV;
    ID3D11ShaderResourceView* m_quadBillboardSRV;
    ID3D11UnorderedAccessView* m_quadBillboardUAV;
    ID3D11ShaderResourceView* m_quadBillboardTextureSRV;

    std::vector<ParticleDataType> m_particlesDataBuffer;
    std::vector<VertexDataType> m_quadBillboardDataBuffer;

    ID3D11SamplerState* m_sampleState;
    std::unique_ptr<TextureClass> m_Texture;
    Vector2 m_MousePosition;
    int m_ScreenWidth;
    int m_ScreenHeight;
};

#endif
