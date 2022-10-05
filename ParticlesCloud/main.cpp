#include "SystemClass.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
    SystemClass System{};
    
    bool result;

    // Initialize and run the system object.
    if (System.Initialize())
    {
        System.Run();
    }

    // Shutdown and release the system object.
    System.Shutdown();

    return 0;
}