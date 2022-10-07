#ifndef _SYSTEMCLASS_H_
#define _SYSTEMCLASS_H_

///////////////////////////////
// PRE-PROCESSING DIRECTIVES //
///////////////////////////////
#define WIN32_LEAN_AND_MEAN

#include <memory>

#include <windows.h>

#include "CpuClass.h"
#include "FpsClass.h"
#include "GraphicsClass.h"
#include "InputClass.h"
#include "TimerClass.h"

class SystemClass
{
public:
    bool Initialize();
    void Shutdown();
    void Run();

    LRESULT CALLBACK MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

private:
    bool Frame();
    void InitializeWindows(int& screenWidth, int& screenHeight);
    void ShutdownWindows();

private:
    LPCWSTR m_applicationName;
    HINSTANCE m_hinstance;
    HWND m_hwnd;

    std::unique_ptr<InputClass> m_Input;
    std::unique_ptr<GraphicsClass> m_Graphics;
    std::unique_ptr<FpsClass> m_Fps;
    std::unique_ptr<CpuClass> m_Cpu;
    std::unique_ptr<TimerClass> m_Timer;
};

/////////////////////////
// FUNCTION PROTOTYPES //
/////////////////////////
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

/////////////
// GLOBALS //
/////////////
static SystemClass* ApplicationHandle = 0;

#endif
