#ifndef _TEXTCLASS_H_
#define _TEXTCLASS_H_

#include <memory>
#include <string_view>

#include "FontClass.h"
#include "FontShaderClass.h"

class TextClass
{
private:
    struct SentenceType
    {
        ID3D11Buffer *vertexBuffer, *indexBuffer;
        int vertexCount, indexCount, maxLength;
        float red, green, blue;
    };

    struct VertexType
    {
        Vector3 position;
        Vector2 texture;
    };

public:
    TextClass();
    TextClass(const TextClass&);
    ~TextClass();

    bool Initialize(
        ID3D11Device* device,
        ID3D11DeviceContext* deviceContext,
        HWND hwnd,
        int screenWidth,
        int screenHeight,
        Matrix baseViewMatrix);
    void Shutdown();
    bool Render(ID3D11DeviceContext* deviceContext, Matrix worldMatrix, Matrix orthoMatrix);

    bool SetFps(int fps, ID3D11DeviceContext* deviceContext);
    bool SetCpu(int cpu, ID3D11DeviceContext* deviceContext);

private:
    bool InitializeSentence(SentenceType** sentence, int maxLength, ID3D11Device* device);
    bool UpdateSentence(
        SentenceType* sentence,
        std::string_view text,
        int positionX,
        int positionY,
        float red,
        float green,
        float blue,
        ID3D11DeviceContext* deviceContext);
    void ReleaseSentence(SentenceType** sentence);
    bool RenderSentence(ID3D11DeviceContext* deviceContext, SentenceType* sentence, Matrix worldMatrix, Matrix orthoMatrix);

private:
    std::unique_ptr<FontClass> m_Font;
    std::unique_ptr<FontShaderClass> m_FontShader;
    int m_screenWidth, m_screenHeight;
    Matrix m_baseViewMatrix;

    SentenceType* m_sentence1;
    SentenceType* m_sentence2;
};

#endif