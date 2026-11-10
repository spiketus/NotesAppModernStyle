#include <windows.h>

bool running = true;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    LRESULT result = 0;
    switch (uMsg) {
        case WM_DESTROY:
            running = false;
            PostQuitMessage(0);
            return 0;
        case WM_CLOSE:
            running = false;
            DestroyWindow(hwnd);
            return 0;

        default:
            result = DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return result;
}


int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd){
    //create window class
    WNDCLASSA windows_class = {};
    windows_class.style = CS_HREDRAW | CS_VREDRAW;
    windows_class.lpszClassName = "Game Window Class";
    windows_class.lpfnWndProc = WindowProc;
    windows_class.hInstance = hInstance;


    //register class
    RegisterClassA(&windows_class);


    //create window
    HWND window = CreateWindowA(windows_class.lpszClassName, "win", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL);

    while(running){
        //input
        MSG message;
        while(PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)){
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }

        //simulate

        //render

    }
}