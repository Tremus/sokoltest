#include "common.h"
#include "sokol_app.h"

extern struct sapp_desc create_sapp_desc();

/*
NOTE: sokol uses mutable global variables
Forcing the app to be single insance is recommended

Below is a neat hack for disabling multiple instances on Windows

On MacOS it's even simpler: you add to the bundle.plist "<key>LSMultipleInstancesProhibited</key><true/>""
We've done this already.

TODO: do the same for Linux
*/
#ifdef _WIN32
#include <windows.h>

int main()
{
    // https://stackoverflow.com/questions/171213/how-to-block-running-two-instances-of-the-same-program
    #define APPLICATION_INSTANCE_MUTEX_NAME "{c05aaf8c-a759-4c8a-b9ee-0b6dafd1388e}"

    //Make sure at most one instance of the tool is running
    HANDLE hMutexOneInstance = CreateMutexA(NULL, TRUE, APPLICATION_INSTANCE_MUTEX_NAME);
    bool bAlreadyRunning = GetLastError() == ERROR_ALREADY_EXISTS;
    if (hMutexOneInstance == NULL || bAlreadyRunning)
    {
        if(hMutexOneInstance)
        {
            ReleaseMutex(hMutexOneInstance);
            CloseHandle(hMutexOneInstance);
        }
        return 1;
    }

    struct sapp_desc my_desc = create_sapp_desc(); 
    sapp_run(&my_desc);
    return 0;
}

#else // MacOS & Linux

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    return create_sapp_desc();
}

#endif

#define SOKOL_APP_IMPL
#include "sokol_app.h"