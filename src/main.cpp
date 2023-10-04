#include "clonegame_lib.h"


//Platform Globals
static bool running = true;

//Platform Functions
bool platformCreateWindow(int width, int height, char* title);
void platformUpdateWindow();

//Windows Platform
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

//Windows Globals
static HWND window;

//Platform Implementations
LRESULT CALLBACK windowsWindowCallback(HWND window, UINT msg, WPARAM wParam, LPARAM lParam){
    LRESULT result = 0;
    switch(msg){
        case WM_CLOSE:{
            running = false;
            break;
        }
        default:{
            result = DefWindowProcA(window, msg, wParam, lParam);
        }
    }

    return result;
}


bool platformCreateWindow(int width, int height, char* title){
    HINSTANCE instance = GetModuleHandleA(0);

    WNDCLASSA wc = {};
    wc.hInstance = instance;
    wc.hIcon = LoadIcon(instance, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = title; //Not the title, unique identifier
    wc.lpfnWndProc = windowsWindowCallback; //Long function pointer to window proc address. Callback for inputs

    if(!RegisterClassA(&wc)){
        return false;
    }

    // WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX
    int dwStyle = WS_OVERLAPPEDWINDOW;

    window = CreateWindowExA(0, title, title, dwStyle, 100, 100, width, height, NULL, NULL, instance, NULL);

    if(window == NULL){
        return false;
    }

    ShowWindow(window, SW_SHOW);

    return true;
}

void platformUpdateWindow(){
    MSG msg;

    while(PeekMessageA(&msg, window, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
        /* code */
    }
    
}

#endif

int main(){

platformCreateWindow(1200, 720, "Celeste Clone");

    while(running){
        //update
        platformUpdateWindow();

        SM_TRACE("TESTING");
        SM_WARN("TESTING");
        SM_ERROR("TESTING");
    }

    return 0;
}