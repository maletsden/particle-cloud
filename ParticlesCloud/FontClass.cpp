#include "FontClass.h"

#include <fstream>

FontClass::FontClass()
    : m_Font(s_FontBufferSize)
    , m_Texture(nullptr)
{
}

FontClass::FontClass(const FontClass& other)
{
}

FontClass::~FontClass()
{
    Shutdown();
}

bool FontClass::Initialize(ID3D11Device* device, std::string_view fontFilename, std::wstring_view textureFilename)
{
    bool result;

    // Load in the text file containing the font data.
    result = LoadFontData(fontFilename);
    if (!result)
    {
        return false;
    }

    // Load the texture that has the font characters on it.
    result = LoadTexture(device, textureFilename);
    if (!result)
    {
        return false;
    }

    return true;
}

void FontClass::Shutdown()
{
    // Release the texture object.
    if (m_Texture)
    {
        m_Texture->Shutdown();
        m_Texture.reset();
    }

    return;
}

bool FontClass::LoadFontData(std::string_view filename)
{
    std::ifstream fin;
    int i;
    char temp;

    // Read in the font size and spacing between chars.
    fin.open(filename.data());
    if (fin.fail())
    {
        return false;
    }

    // Read in the 95 used ascii characters for text.
    for (i = 0; i < s_FontBufferSize; i++)
    {
        fin.get(temp);
        while (temp != ' ')
        {
            fin.get(temp);
        }
        fin.get(temp);
        while (temp != ' ')
        {
            fin.get(temp);
        }

        fin >> m_Font[i].left;
        fin >> m_Font[i].right;
        fin >> m_Font[i].size;

        // Normalize the position to range [0, 1].
        m_Font[i].left /= 0.583984;
        m_Font[i].right /= 0.583984;
    }

    // Close the file.
    fin.close();

    return true;
}

bool FontClass::LoadTexture(ID3D11Device* device, std::wstring_view filename)
{
    bool result;

    // Create the texture object.
    m_Texture = std::make_unique<TextureClass>();
    if (!m_Texture)
    {
        return false;
    }

    // Initialize the texture object.
    result = m_Texture->Initialize(device, filename);
    if (!result)
    {
        return false;
    }

    return true;
}

ID3D11ShaderResourceView* FontClass::GetTexture() noexcept
{
    return m_Texture->GetTexture();
}

void FontClass::BuildVertexArray(void* vertices, std::string_view sentence, float drawX, float drawY)
{
    VertexType* vertexPtr;
    int numLetters, index, i, letter;

    constexpr int lineSize = 16;

    // Coerce the input vertices into a VertexType structure.
    vertexPtr = (VertexType*)vertices;

    // Get the number of letters in the sentence.
    numLetters = sentence.length();

    // Initialize the index to the vertex array.
    index = 0;

    // Draw each letter onto a quad.
    for (i = 0; i < numLetters; i++)
    {
        letter = ((int)sentence[i]) - 32;

        // If the letter is a space then just move over three pixels.
        if (letter == 0)
        {
            drawX = drawX + (3.0f * m_FontSize);
        }
        else
        {
            // First triangle in quad.
            vertexPtr[index].position = { drawX, drawY, 0.0f, 1.0f };  // Top left.
            vertexPtr[index].texture = { m_Font[letter].left, 0.0f };
            index++;

            vertexPtr[index].position = { (drawX + (m_Font[letter].size * m_FontSize)),
                                          (drawY - (lineSize * m_FontSize)),
                                          0.0f,
                                          1.0f };  // Bottom right.
            vertexPtr[index].texture = { m_Font[letter].right, 1.0f };
            index++;

            vertexPtr[index].position = { drawX, (drawY - (lineSize * m_FontSize)), 0.0f, 1.0f };  // Bottom left.
            vertexPtr[index].texture = { m_Font[letter].left, 1.0f };
            index++;

            // Second triangle in quad.
            vertexPtr[index].position = { drawX, drawY, 0.0f, 1.0f };  // Top left.
            vertexPtr[index].texture = { m_Font[letter].left, 0.0f };
            index++;

            vertexPtr[index].position = { drawX + (m_Font[letter].size * m_FontSize), drawY, 0.0f, 1.0f };  // Top right.
            vertexPtr[index].texture = { m_Font[letter].right, 0.0f };
            index++;

            vertexPtr[index].position = { (drawX + (m_Font[letter].size * m_FontSize)),
                                          (drawY - (lineSize * m_FontSize)),
                                          0.0f,
                                          1.0f };  // Bottom right.
            vertexPtr[index].texture = { m_Font[letter].right, 1.0f };
            index++;

            // Update the x location for drawing by the size of the letter and one pixel.
            drawX = drawX + (m_Font[letter].size * m_FontSize) + (1.0f * m_FontSize);
        }
    }
}