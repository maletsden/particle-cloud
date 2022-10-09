#ifndef _INPUTCLASS_H_
#define _INPUTCLASS_H_

///////////////////////////////
// PRE-PROCESSING DIRECTIVES //
///////////////////////////////
#define DIRECTINPUT_VERSION 0x0800

#include <array>

#include <dinput.h>

class InputClass
{
public:
    InputClass();
    InputClass(const InputClass&);
    ~InputClass();

    bool Initialize(HINSTANCE, HWND, int, int);
    void Shutdown();
    bool Frame();

    bool IsEscapePressed() const noexcept;
    void GetMouseLocation(int& mouseX, int& mouseY) const noexcept;

private:
    bool ReadKeyboard();
    bool ReadMouse();
    void ProcessInput() noexcept;

private:
    IDirectInput8* m_directInput;
    IDirectInputDevice8* m_keyboard;
    IDirectInputDevice8* m_mouse;

    std::array<unsigned char, 256> m_keyboardState;
    DIMOUSESTATE m_mouseState;

    int m_screenWidth, m_screenHeight;
    int m_mouseX, m_mouseY;
};

#endif