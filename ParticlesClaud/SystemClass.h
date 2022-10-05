#ifndef _SYSTEMCLASS_H_
#define _SYSTEMCLASS_H_

///////////////////////////////
// PRE-PROCESSING DIRECTIVES //
///////////////////////////////
#define WIN32_LEAN_AND_MEAN

#include <memory>

#include <windows.h>

#include "InputClass.h"
#include "GraphicsClass.h"

class SystemClass {
public:
  bool Initialize();
  void Shutdown();
  void Run();

  LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
  bool Frame();
  void InitializeWindows(int &, int &);
  void ShutdownWindows();

private:
  LPCWSTR m_applicationName;
  HINSTANCE m_hinstance;
  HWND m_hwnd;

  std::unique_ptr<InputClass> m_Input;
  std::unique_ptr<GraphicsClass> m_Graphics;
};

/////////////////////////
// FUNCTION PROTOTYPES //
/////////////////////////
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

/////////////
// GLOBALS //
/////////////
static SystemClass *ApplicationHandle = 0;

#endif
