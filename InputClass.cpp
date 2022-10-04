#include "InputClass.h"

void InputClass::Initialize() noexcept
{
    // Initialize all the keys to being released and not pressed.
    m_keys.fill(false);
}

void InputClass::KeyDown(unsigned int input)
{
    // If a key is pressed then save that state in the key array.
    m_keys[input] = true;
}

void InputClass::KeyUp(unsigned int input)
{
    // If a key is released then clear that state in the key array.
    m_keys[input] = false;
}

bool InputClass::IsKeyDown(unsigned int key) const
{
    // Return what state the key is in (pressed/not pressed).
    return m_keys[key];
}