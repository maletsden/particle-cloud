#ifndef _TEXTURECLASS_H_
#define _TEXTURECLASS_H_

#include <memory>

#include <d3d11.h>
#include <directxtk/DDSTextureLoader.h>
#include <directxtk/WICTextureLoader.h>

class TextureClass
{
public:
    TextureClass();
    ~TextureClass();

    bool Initialize(ID3D11Device* device, const WCHAR* filename);
    void Shutdown();

    ID3D11ShaderResourceView* GetTexture() noexcept;

private:
    ID3D11ShaderResourceView* m_texture;
};

#endif