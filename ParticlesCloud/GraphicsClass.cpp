#include "GraphicsClass.h"

bool GraphicsClass::Initialize(const int screenWidth, const int screenHeight, HWND hwnd)
{
    bool result;

    // Create the Direct3D object.
    m_D3D = std::make_unique<D3DClass>();
    if (!m_D3D)
    {
        return false;
    }

    // Initialize the Direct3D object.
    result = m_D3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
    if (!result)
    {
        MessageBox(hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
        return false;
    }

    // Create the camera object.
    m_Camera = std::make_unique<CameraClass>();
    if (!m_Camera)
    {
        return false;
    }

    // Create the light shader object.
    m_ParticlesShader = std::make_unique<ParticlesShader>();
    if (!m_ParticlesShader)
    {
        return false;
    }

    // Initialize the light shader object.
    result = m_ParticlesShader->Initialize(m_D3D->GetDevice(), hwnd);
    if (!result)
    {
        MessageBox(hwnd, L"Could not initialize the particles shader object.", L"Error", MB_OK);
        return false;
    }

    return true;
}

void GraphicsClass::Shutdown()
{
    // Release the particles shader class.
    if (m_ParticlesShader)
    {
        m_ParticlesShader->Shutdown();
        m_ParticlesShader.reset();
    }

    // Release the camera object.
    m_Camera.reset();

    if (m_D3D)
    {
        m_D3D->Shutdown();
        m_D3D.reset();
    }
    return;
}

bool GraphicsClass::Frame()
{
    bool result;
    constexpr float deltaTime = 0.01f;

    Vector3 cameraPosition = m_Camera->GetPosition();
    m_Camera->SetPosition(cameraPosition.x + deltaTime, 0.0, -70.0);

    // Render the graphics scene.
    result = Render();
    if (!result)
    {
        return false;
    }

    return true;
}

bool GraphicsClass::Render()
{
    Matrix projectionMatrix;

    // Clear the buffers to begin the scene.
    m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // Generate the view matrix based on the camera's position.
    m_Camera->Render();

    // Render the particles.
    const bool result = m_ParticlesShader->Render(m_D3D->GetDeviceContext(), 0, m_Camera->GetViewMatrix(), m_D3D->GetProjectionMatrix());
    if (!result)
    {
        return false;
    }

    // Present the rendered scene to the screen.
    m_D3D->EndScene();

    return true;
}
