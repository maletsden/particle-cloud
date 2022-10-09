#include "InputClass.h"

#include "DirectXUtils.h"

InputClass::InputClass()
    : m_directInput(nullptr)
    , m_keyboard(nullptr)
    , m_mouse(nullptr)
    , m_mouseX(0)
    , m_mouseY(0)
    , m_screenHeight(0)
    , m_screenWidth(0)
    , m_mouseState(DIMOUSESTATE{})
{
    m_keyboardState.fill(0);
}

InputClass::InputClass(const InputClass& other)
{
}

InputClass::~InputClass()
{
}

bool InputClass::Initialize(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight)
{
    HRESULT result;

    // Store the screen size which will be used for positioning the mouse cursor.
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;

    // Initialize the location of the mouse on the screen.
    m_mouseX = 0;
    m_mouseY = 0;

    // Initialize the main direct input interface.
    result = DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, reinterpret_cast<void**>(&m_directInput), nullptr);
    if (FAILED(result))
    {
        return false;
    }

    // Initialize the direct input interface for the keyboard.
    result = m_directInput->CreateDevice(GUID_SysKeyboard, &m_keyboard, NULL);
    if (FAILED(result))
    {
        return false;
    }

    // Set the data format.  In this case since it is a keyboard we can use the predefined data format.
    result = m_keyboard->SetDataFormat(&c_dfDIKeyboard);
    if (FAILED(result))
    {
        return false;
    }

    // Set the cooperative level of the keyboard to not share with other programs.
    result = m_keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
    if (FAILED(result))
    {
        return false;
    }

    // Now acquire the keyboard.
    result = m_keyboard->Acquire();
    if (FAILED(result))
    {
        return false;
    }

    // Initialize the direct input interface for the mouse.
    result = m_directInput->CreateDevice(GUID_SysMouse, &m_mouse, nullptr);
    if (FAILED(result))
    {
        return false;
    }

    // Set the data format for the mouse using the pre-defined mouse data format.
    result = m_mouse->SetDataFormat(&c_dfDIMouse);
    if (FAILED(result))
    {
        return false;
    }

    // Set the cooperative level of the mouse to share with other programs.
    result = m_mouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
    if (FAILED(result))
    {
        return false;
    }

    // Acquire the mouse.
    result = m_mouse->Acquire();
    if (FAILED(result))
    {
        return false;
    }

    return true;
}

void InputClass::Shutdown()
{
    // Release the mouse.
    if (m_mouse)
    {
        m_mouse->Unacquire();
        DirectXUtils::SafeRelease(m_mouse);
    }

    // Release the keyboard.
    if (m_keyboard)
    {
        m_keyboard->Unacquire();
        DirectXUtils::SafeRelease(m_keyboard);
    }

    // Release the main interface to direct input.
    DirectXUtils::SafeRelease(m_directInput);

    return;
}

bool InputClass::Frame()
{
    bool result;

    // Read the current state of the keyboard.
    result = ReadKeyboard();
    if (!result)
    {
        return false;
    }

    // Read the current state of the mouse.
    result = ReadMouse();
    if (!result)
    {
        return false;
    }

    // Process the changes in the mouse and keyboard.
    ProcessInput();

    return true;
}

bool InputClass::ReadKeyboard()
{
    HRESULT result;

    // Read the keyboard device.
    result = m_keyboard->GetDeviceState(sizeof(m_keyboardState), reinterpret_cast<LPVOID>(&m_keyboardState));
    if (FAILED(result))
    {
        // If the keyboard lost focus or was not acquired then try to get control back.
        if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
        {
            m_keyboard->Acquire();
        }
        else
        {
            return false;
        }
    }

    return true;
}
bool InputClass::ReadMouse()
{
    HRESULT result;

    // Read the mouse device.
    result = m_mouse->GetDeviceState(sizeof(DIMOUSESTATE), reinterpret_cast<LPVOID>(&m_mouseState));
    if (FAILED(result))
    {
        // If the mouse lost focus or was not acquired then try to get control back.
        if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
        {
            m_mouse->Acquire();
        }
        else
        {
            return false;
        }
    }

    return true;
}

void InputClass::ProcessInput() noexcept
{
    // Update the location of the mouse cursor based on the change of the mouse location during the frame.
    m_mouseX += m_mouseState.lX;
    m_mouseY += m_mouseState.lY;

    // Ensure the mouse location doesn't exceed the screen width or height.
    m_mouseX = min(max(0, m_mouseX), m_screenWidth);
    m_mouseY = min(max(0, m_mouseY), m_screenHeight);
}

bool InputClass::IsEscapePressed() const noexcept
{
    // Do a bitwise and on the keyboard state to check if the escape key is currently being pressed.
    if (m_keyboardState[DIK_ESCAPE] & 0x80)
    {
        return true;
    }

    return false;
}
void InputClass::GetMouseLocation(int& mouseX, int& mouseY) const noexcept
{
    mouseX = m_mouseX;
    mouseY = m_mouseY;
}
