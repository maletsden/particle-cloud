#include "TextureClass.h"

#include <string_view>

TextureClass::TextureClass()
    : m_texture(nullptr)
{
}

TextureClass::~TextureClass()
{
    Shutdown();
}

bool TextureClass::Initialize(ID3D11Device* device, const WCHAR* filename)
{
    HRESULT result;

    // Load the texture.
    if (std::wstring_view(filename).ends_with(L".dds"))
    {
        result = DirectX::CreateDDSTextureFromFile(device, filename, nullptr, &m_texture);
    }
    else
    {
        result = DirectX::CreateWICTextureFromFile(device, filename, nullptr, &m_texture);
    }
    if (FAILED(result))
    {
        return false;
    }

    return true;
}

void TextureClass::Shutdown()
{
    // Release the texture resource.
    if (m_texture)
    {
        m_texture->Release();
        m_texture = nullptr;
    }

    return;
}

ID3D11ShaderResourceView* TextureClass::GetTexture() noexcept
{
    return m_texture;
}
