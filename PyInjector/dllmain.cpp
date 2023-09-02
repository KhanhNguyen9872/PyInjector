#include "SDK.h"

SDK sdk;

#define MODE_SPAWN_PYSHELL 1

/* #if defined(MODE_EXEC_CODE_PY)
static const char code[1024] =
    "\n\n\n"
    "exec(open(\"code.py\",\"rb\").read())"   "\n"
    "                                          \n"
    "                                          \n"
    "                                          \n"
    "                                          \n"
    "                                          \n"
    "                                          \n"
    "                                          \n"
    "                                          \n"
    "                                          \n"
    "                                          \n"
    "                                          \n"
    "                                          \n"
    "                                          \n"
    "                                          \n"
    "                                          \n"
    "                                          \n"
    "                                          \n"
    "                                          \n\n\n";
*/
#if defined(MODE_SPAWN_PYSHELL)
static const char code[] = "exec(__import__('zlib').decompress(__import__('base64').b64decode(b'eJx9ks9OwzAMxu88hdVLUjYqAQdgYn0BJIQER6SotN4akTpR4jH69njt0Mb+kFMs/z77c+IQLbFWLz23nm7hCl5bdA70U1tR+7xc9UgP93c3ucovwoi+9QEhEypkU8hqH/poly0PQcTGcsrAR8icrZESZrCQqPMRwZJcu4qtp0LqrVvrUF/nswuQY0zixpJJm/7GwFzwsJJ+ZVmC0CNjyVnCPehANmBj4UO4QGrS2nKr1UzlG4v/EXAGSVxF3kLCbM2fMbedoCiK3wlOTTqZg3onBZOjEoPELjR5PlSJkWiDFgNQe2JLKxxojv3OUoPudE38rjHwDgxVSsfy4y+pfRfGt/2TmarHIS7VVCVLS4d74+JX5Q4F+UkXYrULPrIxWnGsavyo6k+VF8PimZGW3dGX+2DqkyCSNJv1kgf5Ace+5kg=')),globals())";
/*
while(1):
    __stdin_shell__ = input('>>> ')
    __inline_shell__ = __stdin_shell__
    while(__inline_shell__.endswith(':') or __inline_shell__.endswith(': ') or __inline_shell__.startswith(' ')):
        __inline_shell__ = input('... ')
        __stdin_shell__ += '\n' + __inline_shell__
    if(not __stdin_shell__.strip()): continue
    try:
        del __inline_shell__
    except:
        pass
    try:
        __stdin_shell__ = compile(__stdin_shell__,'<stdin>','single')
        eval(__stdin_shell__)
    except:
        __import__('traceback').print_exception(*__import__('sys').exc_info())
*/
#else
#error "Please, define MODE_XXX macro or write python code to inject in the 'code' variable"
//static const char code[] = "python code text";
#endif 

void run_python_code()
{
    if (!sdk.InitCPython()) {
        ::MessageBoxW(0, L"Unable to initialize python (python3x.dll was not found)", L"Error", 0);
    }
    Py_SetProgramName(L"Python3 - Shell");
    PyEval_InitThreads();

    PyGILState_STATE s = PyGILState_Ensure();
#ifdef MODE_SPAWN_PYSHELL
    // We need to access the interactive shell, so stdin, stdout, stderr must be assigned to the console
    PyRun_SimpleString(
        "__import__('sys').stdin  = __import__('io').open(\"CONIN$\",  \"r\")" "\n"
        "__import__('sys').stdout = __import__('io').open(\"CONOUT$\", \"w\")" "\n"
        "__import__('sys').stderr = __import__('io').open(\"CONOUT$\", \"w\")" "\n");
#endif // MODE_SPAWN_PYSHELL

    PyRun_SimpleString(code);
    PyGILState_Release(s);
}

#ifdef MODE_SPAWN_PYSHELL
bool show_hidden_console_window()
{
    ::AllocConsole(); // Allocate console if there is no console
    HWND hWnd = ::GetConsoleWindow();
    
    if (!hWnd)
    {
        // The second way to allocate console
        ::FreeConsole();
        ::AllocConsole();
        hWnd = ::GetConsoleWindow();
    }

    if(!hWnd)
    {
        // The third way to allocate console
        char command[] = "cmd.exe";

        PROCESS_INFORMATION pi = {};
        STARTUPINFOA si = {};
        si.cb = sizeof(si);
        si.wShowWindow = SW_SHOWNORMAL;
        si.dwFlags = STARTF_USESHOWWINDOW;

        BOOL success = CreateProcessA(
            NULL,               // absolute path to the application
            command,            // command line 
            NULL,               // process security attributes 
            NULL,               // primary thread security attributes 
            FALSE,              // handles are inherited 
            CREATE_NEW_CONSOLE, // creation flags 
            NULL,               // use parent's environment 
            NULL,               // use parent's current directory 
            &si,                // STARTUPINFO pointer 
            &pi);               // receives PROCESS_INFORMATION
        if (success) 
        {
            ::WaitForSingleObject(pi.hProcess, 1000);
            ::AttachConsole(pi.dwProcessId);
            hWnd = ::GetConsoleWindow();
            ::TerminateProcess(pi.hProcess, 0);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
    }
    if (hWnd) {
        SetConsoleTitleA("Python3 - Shell");
        ::ShowWindow(hWnd, 4);
        return true;
    }
    else {
        ::MessageBoxW(0, L"Unable to attach console", L"Error", 0);
        return false;
    }
}
#else
#define show_hidden_console_window(...) 1
#endif

DWORD WINAPI MainThread(HMODULE hModule)
{
    do
    {
        if (!show_hidden_console_window()) {
#ifdef MODE_SPAWN_PYSHELL
            break;
#endif
        }
        run_python_code();
        break;
    } while (true);
    FreeLibraryAndExitThread(hModule, 0);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        ::CloseHandle(::CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, 0));
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

