#ifndef _FONTCLASS_H_
#define _FONTCLASS_H_

#include <d3d11.h>
#include <directxtk/SimpleMath.h>
#include <memory>
#include <vector>

#include "TextureClass.h"

using namespace DirectX::SimpleMath;

class FontClass
{
private:
    struct FontType
    {
        float left, right;
        int size;
    };

    struct VertexType
    {
        Vector3 position;
        Vector2 texture;
    };

public:
    FontClass();
    FontClass(const FontClass&);
    ~FontClass();

    bool Initialize(ID3D11Device* device, std::string_view fontFilename, std::wstring_view textureFilename);
    void Shutdown();

    ID3D11ShaderResourceView* GetTexture() noexcept;

    void BuildVertexArray(void* vertices, std::string_view sentence, float drawX, float drawY);

private:
    bool LoadFontData(std::string_view filename);
    bool LoadTexture(ID3D11Device*, std::wstring_view);

private:
    static constexpr int s_FontBufferSize = 95;
    float m_FontSize = 4.0;

    std::vector<FontType> m_Font;
    std::unique_ptr<TextureClass> m_Texture;
};

#endif