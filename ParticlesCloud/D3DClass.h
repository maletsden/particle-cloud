#ifndef _D3DCLASS_H_
#define _D3DCLASS_H_

#include <d3d11.h>
#include <d3dcommon.h>
#include <directxtk/SimpleMath.h>
#include <dxgi.h>

using namespace DirectX::SimpleMath;

class D3DClass
{
public:
    D3DClass();
    D3DClass(const D3DClass&);
    ~D3DClass();

    bool Initialize(int, int, bool, HWND, bool, float, float);
    void Shutdown();

    void BeginScene(float, float, float, float);
    void EndScene();

    ID3D11Device* GetDevice() noexcept;
    ID3D11DeviceContext* GetDeviceContext() noexcept;

    const Matrix& GetProjectionMatrix() const noexcept;
    const Matrix& GetWorldMatrix() const noexcept;
    const Matrix& GetOrthoMatrix() const noexcept;

    void TurnZBufferOn();
    void TurnZBufferOff();

    void GetVideoCardInfo(char*, int&);

    void TurnOnAlphaBlending();
    void TurnOffAlphaBlending();

private:
    bool m_vsync_enabled;
    int m_videoCardMemory;
    char m_videoCardDescription[128];
    IDXGISwapChain* m_swapChain;
    ID3D11Device* m_device;
    ID3D11DeviceContext* m_deviceContext;
    ID3D11RenderTargetView* m_renderTargetView;
    ID3D11Texture2D* m_depthStencilBuffer;
    ID3D11DepthStencilState* m_depthStencilState;
    ID3D11DepthStencilView* m_depthStencilView;
    ID3D11RasterizerState* m_rasterState;
    Matrix m_projectionMatrix;
    Matrix m_worldMatrix;
    Matrix m_orthoMatrix;
    ID3D11DepthStencilState* m_depthDisabledStencilState;
    ID3D11BlendState* m_alphaEnableBlendingState;
    ID3D11BlendState* m_alphaDisableBlendingState;
};

#endif