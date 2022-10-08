#include "TextClass.h"

#include <vector>

#include "DirectXUtils.h"

TextClass::TextClass()
    : m_sentence1(nullptr)
    , m_sentence2(nullptr)
{
}

TextClass::TextClass(const TextClass& other)
{
}

TextClass::~TextClass()
{
    Shutdown();
}

bool TextClass::Initialize(
    ID3D11Device* device,
    ID3D11DeviceContext* deviceContext,
    HWND hwnd,
    int screenWidth,
    int screenHeight,
    Matrix baseViewMatrix)
{
    bool result;

    // Store the screen width and height.
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;

    // Store the base view matrix.
    m_baseViewMatrix = baseViewMatrix;

    // Create the font object.
    m_Font = std::make_unique<FontClass>();
    if (!m_Font)
    {
        return false;
    }

    // Initialize the font object.
    result = m_Font->Initialize(device, "./assets/fontdata.txt", PWSTR(L"./assets/font.gif"));
    if (!result)
    {
        MessageBox(hwnd, L"Could not initialize the font object.", L"Error", MB_OK);
        return false;
    }

    // Create the font shader object.
    m_FontShader = std::make_unique<FontShaderClass>();
    if (!m_FontShader)
    {
        return false;
    }

    // Initialize the font shader object.
    result = m_FontShader->Initialize(device, hwnd);
    if (!result)
    {
        MessageBox(hwnd, L"Could not initialize the font shader object.", L"Error", MB_OK);
        return false;
    }

    // Initialize the first sentence.
    result = InitializeSentence(&m_sentence1, 16, device);
    if (!result)
    {
        return false;
    }

    // Now update the sentence vertex buffer with the new string information.
    result = UpdateSentence(m_sentence1, "Hello", 100, 100, 1.0f, 1.0f, 1.0f, deviceContext);
    if (!result)
    {
        return false;
    }

    // Initialize the first sentence.
    result = InitializeSentence(&m_sentence2, 16, device);
    if (!result)
    {
        return false;
    }

    // Now update the sentence vertex buffer with the new string information.
    result = UpdateSentence(m_sentence2, "Goodbye", 100, 200, 1.0f, 1.0f, 0.0f, deviceContext);
    if (!result)
    {
        return false;
    }

    return true;
}

void TextClass::Shutdown()
{
    // Release the first sentence.
    ReleaseSentence(&m_sentence1);

    // Release the second sentence.
    ReleaseSentence(&m_sentence2);

    // Release the font shader object.
    if (m_FontShader)
    {
        m_FontShader->Shutdown();
        m_FontShader.reset();
    }

    // Release the font object.
    if (m_Font)
    {
        m_Font->Shutdown();
        m_Font.reset();
    }

    return;
}

bool TextClass::Render(ID3D11DeviceContext* deviceContext, Matrix worldMatrix, Matrix orthoMatrix)
{
    bool result;

    // Draw the first sentence.
    result = RenderSentence(deviceContext, m_sentence1, worldMatrix, orthoMatrix);
    if (!result)
    {
        return false;
    }

    // Draw the second sentence.
    result = RenderSentence(deviceContext, m_sentence2, worldMatrix, orthoMatrix);
    if (!result)
    {
        return false;
    }

    return true;
}

bool TextClass::InitializeSentence(SentenceType** sentencesPtr, int maxLength, ID3D11Device* device)
{
    std::vector<VertexType> vertices;
    std::vector<unsigned long> indices;

    // VertexType* vertices;
    // unsigned long* indices;
    D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
    HRESULT result;

    // Create a new sentence object.
    *sentencesPtr = new SentenceType;
    if (!*sentencesPtr)
    {
        return false;
    }

    SentenceType* sentence = *sentencesPtr;
    // Initialize the sentence buffers to null.
    sentence->vertexBuffer = 0;
    sentence->indexBuffer = 0;

    // Set the maximum length of the sentence.
    sentence->maxLength = maxLength;

    // Set the number of vertices in the vertex array.
    sentence->vertexCount = 6 * maxLength;

    // Set the number of indexes in the index array.
    sentence->indexCount = sentence->vertexCount;

    // Create the vertex array and initialize vertex array to zeros at first.
    vertices.resize(sentence->vertexCount);

    // Create the index array.
    indices.reserve(sentence->indexCount);

    // Initialize the index array.
    for (int i = 0; i < sentence->indexCount; i++)
    {
        indices.push_back(i);
    }

    // Set up the description of the dynamic vertex buffer.
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * sentence->vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    // Give the subresource structure a pointer to the vertex data.
    vertexData.pSysMem = vertices.data();
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    // Create the vertex buffer.
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &(sentence->vertexBuffer));
    if (FAILED(result))
    {
        return false;
    }

    // Set up the description of the static index buffer.
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * sentence->indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;

    // Give the subresource structure a pointer to the index data.
    indexData.pSysMem = indices.data();
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    // Create the index buffer.
    result = device->CreateBuffer(&indexBufferDesc, &indexData, &(sentence->indexBuffer));
    if (FAILED(result))
    {
        return false;
    }

    return true;
}

bool TextClass::UpdateSentence(
    SentenceType* sentence,
    std::string_view text,
    int positionX,
    int positionY,
    float red,
    float green,
    float blue,
    ID3D11DeviceContext* deviceContext)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    VertexType* verticesPtr;

    // Store the color of the sentence.
    sentence->red = red;
    sentence->green = green;
    sentence->blue = blue;

    // Get the number of letters in the sentence.
    const auto numLetters = text.length();

    // Check for possible buffer overflow.
    if (numLetters > sentence->maxLength)
    {
        return false;
    }

    // Create the vertex array and initialize vertex array to zeros at first.
    std::vector<VertexType> vertices(sentence->vertexCount);

    // Calculate the X and Y pixel position on the screen to start drawing to.
    const auto drawX = static_cast<float>(((m_screenWidth / 2) * -1) + positionX);
    const auto drawY = static_cast<float>((m_screenHeight / 2) - positionY);

    // Use the font class to build the vertex array from the sentence text and sentence draw location.
    m_Font->BuildVertexArray(vertices.data(), text, drawX, drawY);

    // Lock the vertex buffer so it can be written to.
    result = deviceContext->Map(sentence->vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    // Get a pointer to the data in the vertex buffer.
    verticesPtr = reinterpret_cast<VertexType*>(mappedResource.pData);

    // Copy the data into the vertex buffer.
    memcpy(verticesPtr, vertices.data(), (sizeof(VertexType) * sentence->vertexCount));

    // Unlock the vertex buffer.
    deviceContext->Unmap(sentence->vertexBuffer, 0);

    return true;
}

void TextClass::ReleaseSentence(SentenceType** sentencesPtr)
{
    if (*sentencesPtr)
    {
        SentenceType* sentence = *sentencesPtr;
        
        // Release the sentence vertex buffer.
        DirectXUtils::SafeRelease(sentence->vertexBuffer);

        // Release the sentence index buffer.
        DirectXUtils::SafeRelease(sentence->indexBuffer);

        // Release the sentence.
        delete sentence;
        *sentencesPtr = nullptr;
    }

    return;
}

bool TextClass::RenderSentence(ID3D11DeviceContext* deviceContext, SentenceType* sentence, Matrix worldMatrix, Matrix orthoMatrix)
{
    unsigned int stride, offset;
    Vector4 pixelColor;
    bool result;

    // Set vertex buffer stride and offset.
    stride = sizeof(VertexType);
    offset = 0;

    // Set the vertex buffer to active in the input assembler so it can be rendered.
    deviceContext->IASetVertexBuffers(0, 1, &sentence->vertexBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
    deviceContext->IASetIndexBuffer(sentence->indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Create a pixel color vector with the input sentence color.
    pixelColor = { sentence->red, sentence->green, sentence->blue, 1.0f };

    // Render the text using the font shader.
    result =
        m_FontShader
            ->Render(deviceContext, sentence->indexCount, worldMatrix, m_baseViewMatrix, orthoMatrix, m_Font->GetTexture(), pixelColor);
    if (!result)
    {
        false;
    }

    return true;
}

bool TextClass::SetFps(int fps, ID3D11DeviceContext* deviceContext)
{
    char tempString[16];
    char fpsString[16];
    float red, green, blue;
    bool result;

    // Truncate the fps to below 10,000.
    if (fps > 9999)
    {
        fps = 9999;
    }

    // Convert the fps integer to string format.
    _itoa_s(fps, tempString, 10);

    // Setup the fps string.
    strcpy_s(fpsString, "Fps: ");
    strcat_s(fpsString, tempString);

    // If fps is 60 or above set the fps color to green.
    if (fps >= 60)
    {
        red = 0.0f;
        green = 1.0f;
        blue = 0.0f;
    }

    // If fps is below 60 set the fps color to yellow.
    if (fps < 60)
    {
        red = 1.0f;
        green = 1.0f;
        blue = 0.0f;
    }

    // If fps is below 30 set the fps color to red.
    if (fps < 30)
    {
        red = 1.0f;
        green = 0.0f;
        blue = 0.0f;
    }

    // Update the sentence vertex buffer with the new string information.
    result = UpdateSentence(m_sentence1, fpsString, 20, 20, red, green, blue, deviceContext);
    if (!result)
    {
        return false;
    }

    return true;
}

bool TextClass::SetCpu(int cpu, ID3D11DeviceContext* deviceContext)
{
    char tempString[16];
    char cpuString[16];
    bool result;

    // Convert the cpu integer to string format.
    _itoa_s(cpu, tempString, 10);

    // Setup the cpu string.
    strcpy_s(cpuString, "Cpu: ");
    strcat_s(cpuString, tempString);
    strcat_s(cpuString, "%");

    // Update the sentence vertex buffer with the new string information.
    result = UpdateSentence(m_sentence2, cpuString, 20, 160, 0.0f, 1.0f, 0.0f, deviceContext);
    if (!result)
    {
        return false;
    }

    return true;
}
