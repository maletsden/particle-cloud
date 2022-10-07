#ifndef _FONTSHADERCLASS_H_
#define _FONTSHADERCLASS_H_

#include <string_view>

#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxtk/SimpleMath.h>

using namespace DirectX::SimpleMath;

class FontShaderClass
{
private:
    struct ConstantBufferType
    {
        Matrix world;
        Matrix view;
        Matrix projection;
    };

    struct PixelBufferType
    {
        Vector4 pixelColor;
    };

public:
    FontShaderClass();
    FontShaderClass(const FontShaderClass&);
    ~FontShaderClass();

    bool Initialize(ID3D11Device* device, HWND hwnd);
    void Shutdown();
    bool Render(
        ID3D11DeviceContext* deviceContext,
        int indexCount,
        Matrix worldMatrix,
        Matrix viewMatrix,
        Matrix projectionMatrix,
        ID3D11ShaderResourceView* texture,
        Vector4 pixelColor);

private:
    bool InitializeShader(ID3D11Device* device, HWND hwnd, std::wstring_view vsFilename, std::wstring_view psFilename);
    void ShutdownShader();
    void OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, std::wstring_view shaderFilename);

    bool SetShaderParameters(
        ID3D11DeviceContext* deviceContext,
        Matrix worldMatrix,
        Matrix viewMatrix,
        Matrix projectionMatrix,
        ID3D11ShaderResourceView* texture,
        Vector4 pixelColor);
    void RenderShader(ID3D11DeviceContext* deviceContext, int indexCount);

private:
    ID3D11VertexShader* m_vertexShader;
    ID3D11PixelShader* m_pixelShader;
    ID3D11InputLayout* m_layout;
    ID3D11Buffer* m_constantBuffer;
    ID3D11SamplerState* m_sampleState;
    ID3D11Buffer* m_pixelBuffer;
};

#endif
