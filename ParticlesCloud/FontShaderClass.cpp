#include "FontShaderClass.h"

#include <fstream>

#include "DirectXUtils.h"

FontShaderClass::FontShaderClass()
    : m_vertexShader(nullptr)
    , m_pixelShader(nullptr)
    , m_layout(nullptr)
    , m_constantBuffer(nullptr)
    , m_sampleState(nullptr)
    , m_pixelBuffer(nullptr)
{
}

FontShaderClass::FontShaderClass(const FontShaderClass& other)
{
}

FontShaderClass::~FontShaderClass()
{
}

bool FontShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
    bool result;
    // Initialize the vertex and pixel shaders.
    result = InitializeShader(device, hwnd, PWSTR(L"./shaders/fontVS.hlsl"), PWSTR(L"./shaders/fontPS.hlsl"));
    if (!result)
    {
        return false;
    }

    return true;
}

void FontShaderClass::Shutdown()
{
    // Shutdown the vertex and pixel shaders as well as the related objects.
    ShutdownShader();

    return;
}

bool FontShaderClass::Render(
    ID3D11DeviceContext* deviceContext,
    int indexCount,
    Matrix worldMatrix,
    Matrix viewMatrix,
    Matrix projectionMatrix,
    ID3D11ShaderResourceView* texture,
    Vector4 pixelColor)
{
    bool result;

    // Set the shader parameters that it will use for rendering.
    result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture, pixelColor);
    if (!result)
    {
        return false;
    }

    // Now render the prepared buffers with the shader.
    RenderShader(deviceContext, indexCount);

    return true;
}

bool FontShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, std::wstring_view vsFilename, std::wstring_view psFilename)
{
    HRESULT result;
    ID3D10Blob* errorMessage;
    ID3D10Blob* vertexShaderBuffer;
    ID3D10Blob* pixelShaderBuffer;
    D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
    unsigned int numElements;
    D3D11_BUFFER_DESC constantBufferDesc;
    D3D11_SAMPLER_DESC samplerDesc;
    D3D11_BUFFER_DESC pixelBufferDesc;

    // Initialize the pointers this function will use to null.
    errorMessage = nullptr;
    vertexShaderBuffer = nullptr;
    pixelShaderBuffer = nullptr;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    dwShaderFlags |= D3DCOMPILE_DEBUG;
    dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    // Compile the vertex shader code.
    result = D3DCompileFromFile(
        vsFilename.data(),
        nullptr,
        nullptr,
        "FontVertexShader",
        "vs_5_0",
        dwShaderFlags,
        0,
        &vertexShaderBuffer,
        &errorMessage);

    if (FAILED(result))
    {
        // If the shader failed to compile it should have writen something to the error message.
        if (errorMessage)
        {
            OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
        }
        // If there was  nothing in the error message then it simply could not find the shader file itself.
        else
        {
            MessageBox(hwnd, vsFilename.data(), L"Missing Shader File", MB_OK);
        }

        return false;
    }

    // Compile the pixel shader code.
    result = D3DCompileFromFile(
        psFilename.data(),
        nullptr,
        nullptr,
        "FontPixelShader",
        "ps_5_0",
        dwShaderFlags,
        0,
        &pixelShaderBuffer,
        &errorMessage);
    if (FAILED(result))
    {
        // If the shader failed to compile it should have writen something to the error message.
        if (errorMessage)
        {
            OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
        }
        // If there was  nothing in the error message then it simply could not find the file itself.
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

    // Create the vertex shader from the buffer.
    result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
    if (FAILED(result))
    {
        return false;
    }

    // Create the vertex input layout description.
    // This setup needs to match the VertexType stucture in the ModelClass and in the shader.
    polygonLayout[0].SemanticName = "POSITION";
    polygonLayout[0].SemanticIndex = 0;
    polygonLayout[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    polygonLayout[0].InputSlot = 0;
    polygonLayout[0].AlignedByteOffset = 0;
    polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[0].InstanceDataStepRate = 0;

    polygonLayout[1].SemanticName = "TEXCOORD";
    polygonLayout[1].SemanticIndex = 0;
    polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    polygonLayout[1].InputSlot = 0;
    polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[1].InstanceDataStepRate = 0;

    // Get a count of the elements in the layout.
    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

    // Create the vertex input layout.
    result = device->CreateInputLayout(
        polygonLayout,
        numElements,
        vertexShaderBuffer->GetBufferPointer(),
        vertexShaderBuffer->GetBufferSize(),
        &m_layout);
    if (FAILED(result))
    {
        return false;
    }

    // Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
    vertexShaderBuffer->Release();
    vertexShaderBuffer = 0;

    pixelShaderBuffer->Release();
    pixelShaderBuffer = 0;

    // Setup the description of the dynamic constant buffer that is in the vertex shader.
    constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    constantBufferDesc.ByteWidth = sizeof(ConstantBufferType);
    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constantBufferDesc.MiscFlags = 0;
    constantBufferDesc.StructureByteStride = 0;

    // Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
    result = device->CreateBuffer(&constantBufferDesc, NULL, &m_constantBuffer);
    if (FAILED(result))
    {
        return false;
    }

    // Create a texture sampler state description.
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
    result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
    if (FAILED(result))
    {
        return false;
    }

    // Setup the description of the dynamic pixel constant buffer that is in the pixel shader.
    pixelBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    pixelBufferDesc.ByteWidth = sizeof(PixelBufferType);
    pixelBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    pixelBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    pixelBufferDesc.MiscFlags = 0;
    pixelBufferDesc.StructureByteStride = 0;

    // Create the pixel constant buffer pointer so we can access the pixel shader constant buffer from within this class.
    result = device->CreateBuffer(&pixelBufferDesc, NULL, &m_pixelBuffer);
    if (FAILED(result))
    {
        return false;
    }

    return true;
}

void FontShaderClass::ShutdownShader()
{
    DirectXUtils::SafeRelease(m_pixelBuffer);
    DirectXUtils::SafeRelease(m_sampleState);
    DirectXUtils::SafeRelease(m_constantBuffer);
    DirectXUtils::SafeRelease(m_layout);
    DirectXUtils::SafeRelease(m_pixelShader);
    DirectXUtils::SafeRelease(m_vertexShader);
}

void FontShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, std::wstring_view shaderFilename)
{
    char* compileErrors;
    unsigned long bufferSize, i;
    std::ofstream fout;

    // Get a pointer to the error message text buffer.
    compileErrors = reinterpret_cast<char*>(errorMessage->GetBufferPointer());

    // Get the length of the message.
    bufferSize = errorMessage->GetBufferSize();

    // Open a file to write the error message to.
    fout.open("shader-error.txt");

    // Write out the error message.
    for (i = 0; i < bufferSize; i++)
    {
        fout << compileErrors[i];
    }

    // Close the file.
    fout.close();

    // Release the error message.
    errorMessage->Release();
    errorMessage = 0;

    // Pop a message up on the screen to notify the user to check the text file for compile errors.
    MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename.data(), MB_OK);

    return;
}

bool FontShaderClass::SetShaderParameters(
    ID3D11DeviceContext* deviceContext,
    Matrix worldMatrix,
    Matrix viewMatrix,
    Matrix projectionMatrix,
    ID3D11ShaderResourceView* texture,
    Vector4 pixelColor)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ConstantBufferType* dataPtr;
    unsigned int bufferNumber;
    PixelBufferType* dataPtr2;

    // Lock the constant buffer so it can be written to.
    result = deviceContext->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    // Get a pointer to the data in the constant buffer.
    dataPtr = (ConstantBufferType*)mappedResource.pData;

    // Transpose the matrices to prepare them for the shader.
    worldMatrix = worldMatrix.Transpose();
    viewMatrix = viewMatrix.Transpose();
    projectionMatrix = projectionMatrix.Transpose();

    // Copy the matrices into the constant buffer.
    dataPtr->world = worldMatrix;
    dataPtr->view = viewMatrix;
    dataPtr->projection = projectionMatrix;

    // Unlock the constant buffer.
    deviceContext->Unmap(m_constantBuffer, 0);

    // Set the position of the constant buffer in the vertex shader.
    bufferNumber = 0;

    // Now set the constant buffer in the vertex shader with the updated values.
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_constantBuffer);

    // Set shader texture resource in the pixel shader.
    deviceContext->PSSetShaderResources(0, 1, &texture);
    // Lock the pixel constant buffer so it can be written to.
    result = deviceContext->Map(m_pixelBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    // Get a pointer to the data in the pixel constant buffer.
    dataPtr2 = (PixelBufferType*)mappedResource.pData;

    // Copy the pixel color into the pixel constant buffer.
    dataPtr2->pixelColor = pixelColor;

    // Unlock the pixel constant buffer.
    deviceContext->Unmap(m_pixelBuffer, 0);

    // Set the position of the pixel constant buffer in the pixel shader.
    bufferNumber = 0;

    // Now set the pixel constant buffer in the pixel shader with the updated value.
    deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_pixelBuffer);

    return true;
}

void FontShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
    // Set the vertex input layout.
    deviceContext->IASetInputLayout(m_layout);

    // Set the vertex and pixel shaders that will be used to render the triangles.
    deviceContext->VSSetShader(m_vertexShader, nullptr, 0);
    deviceContext->PSSetShader(m_pixelShader, nullptr, 0);

    // Set the sampler state in the pixel shader.
    deviceContext->PSSetSamplers(0, 1, &m_sampleState);

    // Render the triangles.
    deviceContext->DrawIndexed(indexCount, 0, 0);

    return;
}
