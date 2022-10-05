#ifndef _INPUTCLASS_H_
#define _INPUTCLASS_H_

#include <array>

class InputClass
{
public:
    void Initialize() noexcept;

    void KeyDown(unsigned int input);
    void KeyUp(unsigned int input);

    bool IsKeyDown(unsigned int input) const;

private:
    std::array<bool, 256> m_keys;
};

#endif