#include "../include/PostIt.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Suprimir warnings de parámetros no utilizados
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;
    
    PostItManager manager(hInstance);
    
    if (!manager.Initialize()) {
        MessageBoxW(nullptr, L"Error al inicializar la aplicación", L"Error", MB_OK | MB_ICONERROR);
        return -1;
    }
    
    manager.Run();
    
    return 0;
}
