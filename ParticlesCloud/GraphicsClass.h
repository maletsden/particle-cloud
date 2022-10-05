#ifndef _GRAPHICSCLASS_H_
#define _GRAPHICSCLASS_H_

#include <memory>

#include <windows.h>

#include "CameraClass.h"
#include "D3DClass.h"
#include "ParticlesShader.h"

constexpr bool FULL_SCREEN = true;
constexpr bool VSYNC_ENABLED = true;
constexpr float SCREEN_DEPTH = 1000.0f;
constexpr float SCREEN_NEAR = 0.1f;

class GraphicsClass
{
public:
    bool Initialize(const int screenWidth, const int screenHeight, HWND hwnd);
    void Shutdown();
    bool Frame();

private:
    bool Render();

private:
    std::unique_ptr<D3DClass> m_D3D;
    std::unique_ptr<CameraClass> m_Camera;
    std::unique_ptr<ParticlesShader> m_ParticlesShader;
};

#endif