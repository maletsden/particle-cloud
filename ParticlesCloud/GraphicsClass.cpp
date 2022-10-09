#include "GraphicsClass.h"

GraphicsClass::GraphicsClass() = default;

GraphicsClass::~GraphicsClass()
{
    Shutdown();
}

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

    // Initialize a base view matrix with the camera for 2D user interface rendering.
    m_Camera->SetPosition(0.0f, 0.0f, -1.0f);
    m_Camera->Render();
    const Matrix& baseViewMatrix = m_Camera->GetViewMatrix();

    // Create the text object.
    m_Text = std::make_unique<TextClass>();
    if (!m_Text)
    {
        return false;
    }

    // Initialize the text object.
    result = m_Text->Initialize(m_D3D->GetDevice(), m_D3D->GetDeviceContext(), hwnd, screenWidth, screenHeight, baseViewMatrix);
    if (!result)
    {
        MessageBox(hwnd, L"Could not initialize the text object.", L"Error", MB_OK);
        return false;
    }

    // Create the light shader object.
    m_ParticlesShader = std::make_unique<ParticlesShader>();
    if (!m_ParticlesShader)
    {
        return false;
    }

    // Initialize the light shader object.
    result = m_ParticlesShader->Initialize(m_D3D->GetDevice(), hwnd, screenWidth, screenHeight);
    if (!result)
    {
        MessageBox(hwnd, L"Could not initialize the particles shader object.", L"Error", MB_OK);
        return false;
    }

    return true;
}

void GraphicsClass::Shutdown()
{
    // Release the text object.
    if (m_Text)
    {
        m_Text->Shutdown();
        m_Text.reset();
    }

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

bool GraphicsClass::Frame(int fps, int cpu, float frameTime, int mouseX, int mouseY)
{
    bool result;
    constexpr float deltaTime = 0.01f;

    // Set the frames per second.
     result = m_Text->SetFps(fps, m_D3D->GetDeviceContext());
    if (!result)
    {
        return false;
    }

    // Set the cpu usage.
     result = m_Text->SetCpu(cpu, m_D3D->GetDeviceContext());
    if (!result)
    {
        return false;
    }

    Vector3 cameraPosition = m_Camera->GetPosition();
    m_Camera->SetPosition(cameraPosition.x + deltaTime, 0.0, -70.0);

    m_ParticlesShader->SetMousePosition(Vector2(mouseX, mouseY));

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
    bool result;

    // Clear the buffers to begin the scene.
    m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // Generate the view matrix based on the camera's position.
    m_Camera->Render();

    // Render the particles.
    result = m_ParticlesShader->Render(m_D3D->GetDeviceContext(), 0, m_Camera->GetViewMatrix(), m_D3D->GetProjectionMatrix());
    if (!result)
    {
        return false;
    }

    const auto& viewMatrix = m_Camera->GetViewMatrix();
    const auto& worldMatrix = m_D3D->GetWorldMatrix();
    // const auto projectionMatrix = m_D3D->GetProjectionMatrix();
    const auto& orthoMatrix = m_D3D->GetOrthoMatrix();

    // Turn off the Z buffer to begin all 2D rendering.
    m_D3D->TurnZBufferOff();

    // Turn on the alpha blending before rendering the text.
    m_D3D->TurnOnAlphaBlending();

    // Render the text strings.
    result = m_Text->Render(m_D3D->GetDeviceContext(), worldMatrix, orthoMatrix);
    if (!result)
    {
        return false;
    }

    // Turn off alpha blending after rendering the text.
    m_D3D->TurnOffAlphaBlending();

    // Turn the Z buffer back on now that all 2D rendering has completed.
    m_D3D->TurnZBufferOn();

    // Present the rendered scene to the screen.
    m_D3D->EndScene();

    return true;
}
