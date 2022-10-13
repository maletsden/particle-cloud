#define NOMINMAX

#include "ParticlesShader.h"

#include <algorithm>
#include <fstream>
#include <random>

#include "DirectXUtils.h"

ParticlesShader::ParticlesShader()
    : m_vertexShader(nullptr)
    , m_pixelShader(nullptr)
    , m_computeShader(nullptr)
    , m_sampleState(nullptr)
    , m_csParametersBuffer(nullptr)
    , m_particlesBuffer(nullptr)
    , m_particlesUAV(nullptr)
    , m_particlesSRV(nullptr)
    , m_ScreenWidth(0)
    , m_ScreenHeight(0)
    , m_lastSampleTime(std::chrono::high_resolution_clock::time_point::max())
{
    std::uniform_real_distribution<float> positionDistribution(-25.5f, 25.5f);
    std::default_random_engine generator;

    m_particlesDataBuffer.reserve(s_ParticlesNumber);
    for (int i = 0; i < s_ParticlesNumber; ++i)
    {
        m_particlesDataBuffer.push_back(ParticleDataType{
            Vector4{ positionDistribution(generator), positionDistribution(generator), positionDistribution(generator), 1.0f },
            Vector4{ 0.0f, 0.0f, 0.0f, 0.0f },
            Vector3{ 0.0f, 0.0f, 0.0f },
            0.0f
        });
    }
}

ParticlesShader::~ParticlesShader()
{
    Shutdown();
}

bool ParticlesShader::Initialize(ID3D11Device* device, HWND hwnd, const int screenWidth, const int screenHeight)
{
    bool result;

    // Initialize the vertex and pixel shaders.
    result = InitializeShader(
        device,
        hwnd,
        PWSTR(L"./shaders/particlesVS.hlsl"),
        PWSTR(L"./shaders/particlesPS.hlsl"),
        PWSTR(L"./shaders/particlesCS.hlsl"));
    if (!result)
    {
        return false;
    }

    // Initialize billboards texture.
    result = InitializeTexture(device, PWSTR(L"./assets/blue_texture.jpg"));

    m_ScreenWidth = screenWidth;
    m_ScreenHeight = screenHeight;

    return true;
}

void ParticlesShader::Shutdown()
{
    ShutdownShader();
}

bool ParticlesShader::Render(ID3D11DeviceContext* deviceContext, int indexCount, const Matrix& viewMatrix, const Matrix& projectionMatrix)
{
    bool result;

    result = UpdateFrameDeltaTime();
    if (!result)
    {
        return false;
    }

    result = UpdateGravityFieldPosition(viewMatrix, projectionMatrix);
    if (!result)
    {
        return false;
    }

    result = UpdateTransformationMatrices(viewMatrix, projectionMatrix);
    if (!result)
    {
        return false;
    }

    // Set the shader parameters that it will use for rendering.
    result = SetShaderParameters(deviceContext, m_Texture->GetTexture());
    if (!result)
    {
        return false;
    }

    // Now render the prepared buffers with the shader.
    RenderShader(deviceContext, indexCount);

    return true;
}

bool ParticlesShader::InitializeShader(
    ID3D11Device* device,
    HWND hwnd,
    std::wstring_view vsFilename,
    std::wstring_view psFilename,
    std::wstring_view csFilename)
{
    HRESULT result;
    ID3D10Blob* errorMessage;
    ID3D10Blob* vertexShaderBuffer;
    ID3D10Blob* pixelShaderBuffer;

    D3D11_BUFFER_DESC matrixBufferDesc;

    // Initialize the pointers this function will use to null.
    errorMessage = 0;
    vertexShaderBuffer = 0;
    pixelShaderBuffer = 0;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    dwShaderFlags |= D3DCOMPILE_DEBUG;
    dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    // Compile the vertex shader code.
    result =
        D3DCompileFromFile(vsFilename.data(), nullptr, nullptr, "ParticleVS", "vs_5_0", dwShaderFlags, 0, &vertexShaderBuffer, &errorMessage);
    if (FAILED(result))
    {
        // If the shader failed to compile it should have writen something to the
        // error message.
        if (errorMessage)
        {
            OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
        }
        // If there was nothing in the error message then it simply could not find
        // the shader file itself.
        else
        {
            MessageBox(hwnd, vsFilename.data(), L"Missing Shader File", MB_OK);
        }

        return false;
    }

    // Compile the pixel shader code.
    result =
        D3DCompileFromFile(psFilename.data(), nullptr, nullptr, "ParticlePS", "ps_5_0", dwShaderFlags, 0, &pixelShaderBuffer, &errorMessage);
    if (FAILED(result))
    {
        // If the shader failed to compile it should have writen something to the
        // error message.
        if (errorMessage)
        {
            OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
        }
        // If there was nothing in the error message then it simply could not find
        // the file itself.
        else
        {
            MessageBox(hwnd, psFilename.data(), L"Missing Shader File", MB_OK);
        }

        return false;
    }

    // Create the vertex shader from the buffer.
    result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
    if (FAILED(result))
    {
        return false;
    }

    // Create the pixel shader from the buffer.
    result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
    if (FAILED(result))
    {
        return false;
    }

    // Release the vertex shader buffer and pixel shader buffer since they are no
    // longer needed.
    DirectXUtils::SafeRelease(vertexShaderBuffer);
    DirectXUtils::SafeRelease(pixelShaderBuffer);

    result = DirectXUtils::CreateDynamicConstantBuffer(device, sizeof(CSParametersBufferType), &m_csParametersBuffer);
    if (FAILED(result))
    {
        return false;
    }

    result = DirectXUtils::CreateStructuredBuffer(
        device,
        sizeof(ParticleDataType),
        m_particlesDataBuffer.size(),
        m_particlesDataBuffer.data(),
        &m_particlesBuffer);
    if (FAILED(result))
    {
        return false;
    }

    result = DirectXUtils::CreateBufferUAV(device, m_particlesBuffer, &m_particlesUAV);
    if (FAILED(result))
    {
        return false;
    }

    result = DirectXUtils::CreateBufferSRV(device, m_particlesBuffer, &m_particlesSRV);
    if (FAILED(result))
    {
        return false;
    }

    const auto computeShaderInitResult = InitializeComputeShader(device, hwnd, csFilename);

    if (!computeShaderInitResult)
    {
        return false;
    }

    return true;
}

bool ParticlesShader::InitializeTexture(ID3D11Device* device, std::wstring_view textureFilename)
{
    bool result;

    // Create the texture object.
    m_Texture = std::make_unique<TextureClass>();
    if (!m_Texture)
    {
        return false;
    }

    // Initialize the texture object.
    result = m_Texture->Initialize(device, textureFilename.data());
    if (!result)
    {
        return false;
    }

    // Create a texture sampler state description.
    D3D11_SAMPLER_DESC samplerDesc{};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0;
    samplerDesc.BorderColor[1] = 0;
    samplerDesc.BorderColor[2] = 0;
    samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    // Create the texture sampler state.
    const auto samplerResult = device->CreateSamplerState(&samplerDesc, &m_sampleState);
    if (FAILED(samplerResult))
    {
        return false;
    }

    return true;
}

void ParticlesShader::ShutdownShader()
{
    // Release the texture object.
    if (m_Texture)
    {
        m_Texture->Shutdown();
        m_Texture.reset();
    }

    DirectXUtils::SafeRelease(m_particlesBuffer);
    DirectXUtils::SafeRelease(m_csParametersBuffer);
    DirectXUtils::SafeRelease(m_sampleState);
    DirectXUtils::SafeRelease(m_pixelShader);
    DirectXUtils::SafeRelease(m_vertexShader);
    DirectXUtils::SafeRelease(m_computeShader);
    DirectXUtils::SafeRelease(m_particlesUAV);
    DirectXUtils::SafeRelease(m_particlesSRV);
}

void ParticlesShader::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, std::wstring_view shaderFilename)
{
    // Get a pointer to the error message text buffer.
    const char* compileErrors = reinterpret_cast<char*>(errorMessage->GetBufferPointer());

    // Get the length of the message.
    const size_t bufferSize = errorMessage->GetBufferSize();

    // Open a file to write the error message to.
    std::ofstream fout{ "shader-error.txt" };

    // Write out the error message.
    for (long i = 0; i < static_cast<long>(bufferSize); i++)
    {
        fout << compileErrors[i];
    }

    // Close the file.
    fout.close();

    // Release the error message.
    errorMessage->Release();
    errorMessage = nullptr;

    // Pop a message up on the screen to notify the user to check the text file
    // for compile errors.
    MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename.data(), MB_OK);

    return;
}

bool ParticlesShader::SetShaderParameters(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* texture)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    CSParametersBufferType* dataPtr;

    // Lock the constant buffer so it can be written to.
    result = deviceContext->Map(m_csParametersBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    // Get a pointer to the data in the constant buffer.
    dataPtr = reinterpret_cast<CSParametersBufferType*>(mappedResource.pData);

    *dataPtr = m_CSParameters;

    // Unlock the constant buffer.
    deviceContext->Unmap(m_csParametersBuffer, 0);

    // Now set the constant buffer in the compute shader with the updated values.
    deviceContext->CSSetConstantBuffers(0, 1, &m_csParametersBuffer);

    // Set shader texture resource in the pixel shader.
    deviceContext->PSSetShaderResources(0, 1, &texture);

    return true;
}

void ParticlesShader::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
    RunComputeShader(deviceContext);

    unsigned int stride;
    unsigned int offset;

    // Set vertex buffer stride and offset.
    stride = 0;
    offset = 0;

    deviceContext->VSSetShaderResources(0, 1, &m_particlesSRV);

    // Set the vertex and pixel shaders that will be used to render this triangle.
    deviceContext->VSSetShader(m_vertexShader, nullptr, 0);
    deviceContext->PSSetShader(m_pixelShader, nullptr, 0);

    // Set the sampler state in the pixel shader.
    deviceContext->PSSetSamplers(0, 1, &m_sampleState);

    deviceContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);

    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Render the triangles.
    deviceContext->Draw(s_VertixIndeciesNumber, 0);
}

bool ParticlesShader::InitializeComputeShader(ID3D11Device* device, HWND hwnd, std::wstring_view filename)
{
    if (!device || !hwnd)
    {
        return false;
    }

    HRESULT result;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    dwShaderFlags |= D3DCOMPILE_DEBUG;
    dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    // We generally prefer to use the higher CS shader profile when possible as CS 5.0 is better performance on 11-class hardware
    LPCSTR pProfile = (device->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0) ? "cs_5_0" : "cs_4_0";

    ID3D10Blob* errorMessage = nullptr;
    ID3D10Blob* computeShaderBuffer = nullptr;

    result = D3DCompileFromFile(
        filename.data(),
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "DefaultCS",
        pProfile,
        dwShaderFlags,
        0,
        &computeShaderBuffer,
        &errorMessage);

    if (FAILED(result))
    {
        // If the shader failed to compile it should have writen something to the
        // error message.
        if (errorMessage)
        {
            OutputShaderErrorMessage(errorMessage, hwnd, filename);
        }
        // If there was nothing in the error message then it simply could not find
        // the file itself.
        else
        {
            MessageBox(hwnd, filename.data(), L"Missing Shader File", MB_OK);
        }

        return false;
    }
    result = device->CreateComputeShader(
        computeShaderBuffer->GetBufferPointer(),
        computeShaderBuffer->GetBufferSize(),
        nullptr,
        &m_computeShader);

    DirectXUtils::SafeRelease(computeShaderBuffer);
    DirectXUtils::SafeRelease(errorMessage);

    return true;
}

void ParticlesShader::RunComputeShader(ID3D11DeviceContext* deviceContext)
{
    deviceContext->CSSetShader(m_computeShader, nullptr, 0);
    ID3D11UnorderedAccessView* views[1] = { m_particlesUAV };
    deviceContext->CSSetUnorderedAccessViews(0, 1, views, nullptr);

    constexpr size_t threadGroupSize = 1024;
    const auto numGroups = (m_particlesDataBuffer.size() % threadGroupSize != 0) ? ((m_particlesDataBuffer.size() / threadGroupSize) + 1)
                                                                                 : (m_particlesDataBuffer.size() / threadGroupSize);
    const auto secondRoot = std::ceil(std::sqrt(static_cast<double>(numGroups)));
    const auto groupSizeX = static_cast<int>(secondRoot);
    const auto groupSizeY = static_cast<int>(secondRoot);

    deviceContext->Dispatch(groupSizeX, groupSizeY, 1);

    deviceContext->CSSetShader(nullptr, nullptr, 0);

    ID3D11UnorderedAccessView* ppUAViewnullptr[1] = { nullptr };
    deviceContext->CSSetUnorderedAccessViews(0, 1, ppUAViewnullptr, nullptr);
}

bool ParticlesShader::UpdateGravityFieldPosition(const Matrix& viewMatrix, const Matrix& projectionMatrix)
{
    constexpr float circleRadius = 0.5f;
    static float rotation = 0.0f;
    static float positionX = 0.0f;

    Matrix projectionInv = projectionMatrix.Invert();
    Matrix viewInv = viewMatrix.Invert();

    // 3 coordinates that defines XY-plane (in world coordinates).
    Vector4 worldO = { 0, 0, 0, 1 };
    Vector4 worldX = { 1, 0, 0, 1 };
    Vector4 worldY = { 0, 1, 0, 1 };

    // 3 coordinates that defines XY-plane (in camera coordinates).
    Vector4 worldOInCamera = Vector4::Transform(worldO, viewMatrix);
    Vector4 worldXInCamera = Vector4::Transform(worldX, viewMatrix);
    Vector4 worldYInCamera = Vector4::Transform(worldY, viewMatrix);

    // Camera position (in camera coordinates).
    Vector4 cO = { 0, 0, 0, 1 };

    // Normalize mouse position to range [-1, 1].
    Vector4 mousePositionNormalize = Vector4(
        (m_MousePosition.x / static_cast<float>(m_ScreenWidth)) * 2.0f - 1.0,
        1.0 - (m_MousePosition.y / static_cast<float>(m_ScreenHeight)) * 2.0f,
        0,
        1);

    // Unproject point (in camera coordinates).
    Vector3 unprojectedDirection = Vector3(Vector4::Transform(mousePositionNormalize, projectionInv));

    // Define equation Ax = b to calculate the intersection of "unprojectedDirection" and XY world plane.
    Matrix A = Matrix{
        Vector3{ worldXInCamera - worldOInCamera },
        Vector3{ worldYInCamera - worldOInCamera },
        -unprojectedDirection,
    };
    Vector3 b = Vector3(cO - worldOInCamera);

    // Calculate solution for x.
    Vector3 solution = Vector3::Transform(b, A.Invert());

    Vector3 resultPositionInCamera = cO + unprojectedDirection * solution.z;
    Vector4 resultPositionInWorld =
        Vector4::Transform(Vector4(resultPositionInCamera.x, resultPositionInCamera.y, resultPositionInCamera.z, 1.0f), viewInv);

    // Add circular rotation.
    positionX += m_CSParameters.DeltaTime;
    rotation += 1.f * m_CSParameters.DeltaTime;
    const auto theta = DirectX::XMConvertToRadians(rotation);

    m_CSParameters.GravityFieldPosition = Vector3(
        resultPositionInWorld.x,
        resultPositionInWorld.y + circleRadius * std::cos(theta),
        resultPositionInWorld.z + circleRadius * std::sin(theta));

    return true;
}

void ParticlesShader::SetMousePosition(const Vector2& mousePosition) noexcept
{
    m_MousePosition = mousePosition;
}

bool ParticlesShader::UpdateFrameDeltaTime() noexcept
{
    const auto now = std::chrono::high_resolution_clock::now();
    const auto deltaTimeInMilliseconds =
        std::max(std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastSampleTime).count(), 0LL);

    // Set delta time.
    constexpr auto timeScale = 1.f / 15.f;
    m_CSParameters.DeltaTime = static_cast<float>(deltaTimeInMilliseconds) * timeScale;

    m_lastSampleTime = now;

    return true;
}

bool ParticlesShader::UpdateTransformationMatrices(const Matrix& viewMatrix, const Matrix& projectionMatrix) noexcept
{
    // Transpose the matrices to prepare them for the shader. Copy the matrices into the constant buffer.
    m_CSParameters.View = viewMatrix.Transpose();
    m_CSParameters.Projection = projectionMatrix.Transpose();

    return true;
}