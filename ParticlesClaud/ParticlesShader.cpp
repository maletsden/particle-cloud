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
    , m_matrixBuffer(nullptr)
    , m_particlesBuffer(nullptr)
    , m_quadBillboardBuffer(nullptr)
    , m_quadBillboardSRV(nullptr)
    , m_quadBillboardUAV(nullptr)
    , m_particlesUAV(nullptr)
    , m_quadBillboardTextureSRV(nullptr)
{
    std::uniform_real_distribution<float> distribution(-5.0f, 5.0f);
    std::default_random_engine generator;

    m_particlesDataBuffer.reserve(s_ParticlesNumber);
    for (int i = 0; i < s_ParticlesNumber; ++i)
    {
        m_particlesDataBuffer.push_back(ParticleDataType{
            Vector3{ distribution(generator), distribution(generator), distribution(generator) },
            Vector3{ distribution(generator), distribution(generator), distribution(generator) },
        });
    }

    m_quadBillboardDataBuffer.resize(s_ParticlesNumber * 6);
}

ParticlesShader::~ParticlesShader()
{
    Shutdown();
}

bool ParticlesShader::Initialize(ID3D11Device* device, HWND hwnd)
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

    return true;
}

void ParticlesShader::Shutdown()
{
    ShutdownShader();
}

bool ParticlesShader::Render(ID3D11DeviceContext* deviceContext, int indexCount, Matrix viewMatrix, Matrix projectionMatrix)
{
    // Set the shader parameters that it will use for rendering.
    const bool result = SetShaderParameters(deviceContext, viewMatrix, projectionMatrix, m_Texture->GetTexture());
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

    // Compile the vertex shader code.
    result = D3DCompileFromFile(
        vsFilename.data(),
        NULL,
        NULL,
        "ParticleVS",
        "vs_5_0",
        D3D10_SHADER_ENABLE_STRICTNESS,
        0,
        &vertexShaderBuffer,
        &errorMessage);
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
    result = D3DCompileFromFile(
        psFilename.data(),
        NULL,
        NULL,
        "ParticlePS",
        "ps_5_0",
        D3D10_SHADER_ENABLE_STRICTNESS,
        0,
        &pixelShaderBuffer,
        &errorMessage);
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

    // Setup the description of the dynamic matrix constant buffer that is in the
    // vertex shader.
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;

    // Create the constant buffer pointer so we can access the vertex shader
    // constant buffer from within this class.
    result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
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

    result = DirectXUtils::CreateStructuredBuffer(
        device,
        sizeof(VertexDataType),
        m_quadBillboardDataBuffer.size(),
        m_quadBillboardDataBuffer.data(),
        &m_quadBillboardBuffer);
    if (FAILED(result))
    {
        return false;
    }

    result = DirectXUtils::CreateBufferUAV(device, m_particlesBuffer, &m_particlesUAV);
    if (FAILED(result))
    {
        return false;
    }

    result = DirectXUtils::CreateBufferSRV(device, m_quadBillboardBuffer, &m_quadBillboardSRV);
    if (FAILED(result))
    {
        return false;
    }

    result = DirectXUtils::CreateBufferUAV(device, m_quadBillboardBuffer, &m_quadBillboardUAV);
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
    DirectXUtils::SafeRelease(m_quadBillboardBuffer);
    DirectXUtils::SafeRelease(m_matrixBuffer);
    DirectXUtils::SafeRelease(m_sampleState);
    DirectXUtils::SafeRelease(m_pixelShader);
    DirectXUtils::SafeRelease(m_vertexShader);
    DirectXUtils::SafeRelease(m_computeShader);
    DirectXUtils::SafeRelease(m_particlesUAV);
    DirectXUtils::SafeRelease(m_quadBillboardSRV);
    DirectXUtils::SafeRelease(m_quadBillboardUAV);
    DirectXUtils::SafeRelease(m_quadBillboardTextureSRV);
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

bool ParticlesShader::SetShaderParameters(
    ID3D11DeviceContext* deviceContext,
    Matrix viewMatrix,
    Matrix projectionMatrix,
    ID3D11ShaderResourceView* texture)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    unsigned int bufferNumber;
    MatrixBufferType* dataPtr;
    // CameraBufferType* dataPtr3;
    // LightBufferType* dataPtr2;

    // Transpose the matrices to prepare them for the shader.
    viewMatrix = viewMatrix.Transpose();
    projectionMatrix = projectionMatrix.Transpose();

    // Lock the constant buffer so it can be written to.
    result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    // Get a pointer to the data in the constant buffer.
    dataPtr = (MatrixBufferType*)mappedResource.pData;

    // Copy the matrices into the constant buffer.
    dataPtr->view = viewMatrix;
    dataPtr->projection = projectionMatrix;

    // Unlock the constant buffer.
    deviceContext->Unmap(m_matrixBuffer, 0);

    // Set the position of the constant buffer in the vertex shader.
    bufferNumber = 0;

    // Now set the constant buffer in the vertex shader with the updated values.
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

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

    deviceContext->VSSetShaderResources(0, 1, &m_quadBillboardSRV);

    // Set the vertex and pixel shaders that will be used to render this triangle.
    deviceContext->VSSetShader(m_vertexShader, nullptr, 0);
    deviceContext->PSSetShader(m_pixelShader, nullptr, 0);

    // Set the sampler state in the pixel shader.
    deviceContext->PSSetSamplers(0, 1, &m_sampleState);

    deviceContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);

    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Render the triangle.
    deviceContext->Draw(m_quadBillboardDataBuffer.size(), 0);
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
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows
    // the shaders to be optimized and to run exactly the way they will run in
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;

    // Disable optimizations to further improve shader debugging
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
    ID3D11UnorderedAccessView* views[2] = { m_particlesUAV, m_quadBillboardUAV };
    deviceContext->CSSetUnorderedAccessViews(0, 2, views, nullptr);
    deviceContext->CSSetConstantBuffers(0, 1, &m_matrixBuffer);

    deviceContext->Dispatch(m_particlesDataBuffer.size(), 1, 1);

    deviceContext->CSSetShader(nullptr, nullptr, 0);

    ID3D11UnorderedAccessView* ppUAViewnullptr[2] = { nullptr, nullptr };
    deviceContext->CSSetUnorderedAccessViews(0, 2, ppUAViewnullptr, nullptr);

    ID3D11Buffer* ppCBnullptr[1] = { nullptr };
    deviceContext->CSSetConstantBuffers(0, 1, ppCBnullptr);
}
