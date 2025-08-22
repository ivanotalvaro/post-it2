    #pragma once

#include <windows.h>
#include <string>
#include <vector>

// Forward declaration
class PostItManager;

// Estructura para almacenar datos de un Post-it
struct PostItData {
    std::wstring title;
    std::wstring text;
    COLORREF color;
    int transparency;
    RECT position;
    bool isVisible;
    HWND hwnd;
    PostItManager* manager;
    
    PostItData() : color(RGB(255, 255, 0)), transparency(200), 
                   isVisible(true), hwnd(nullptr), manager(nullptr) {
        position = {100, 100, 320, 270}; // Ventana un poco más alta
    }
};

class PostItManager {
private:
    std::vector<PostItData> postIts;
    HWND mainWindow;
    HINSTANCE hInstance;
    
    // Cache de recursos
    HICON appIcon;
    HBRUSH backgroundBrush;
    
    // Configuración
    static const int MAX_POSTITS = 100;
    static const int AUTO_SAVE_INTERVAL = 5000; // 5 segundos
    static const int UPDATE_LIST_DELAY = 500; // 500ms para actualizar lista
    UINT_PTR autoSaveTimer;
    UINT_PTR updateListTimer;
    
    // Procedimientos de ventana
    static LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK PostItWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    
    // Métodos auxiliares mejorados
    void CreateMainWindow();
    // void CreateFloatingMenu(); // Menú flotante eliminado
    HWND CreatePostItWindow(PostItData& postIt);
    void ShowColorPicker(PostItData& postIt);
    void ShowTransparencySlider(PostItData& postIt);
    void UpdateMainWindowList();
    void SavePostIts();
    void LoadPostIts();
    void CreateSystemTrayIcon();
    void RemoveSystemTrayIcon();
    void ShowTrayContextMenu(HWND hwnd, POINT pt);
    
    // Nuevos métodos de optimización
    void AutoSave();
    void CleanupResources();
    void DeletePostIt(int index);
    void HideAllPostIts();
    void ShowAllPostIts();
    void MinimizeToTray();
    bool ValidatePostItIndex(int index);
    void OptimizeLayout();
    
public:
    PostItManager(HINSTANCE hInst);
    ~PostItManager();
    
    bool Initialize();
    void Run();
    void CreateNewPostIt();
    void ShowPostIt(int index);
    void HidePostIt(int index);
    
    // Constantes para IDs de controles
    static const int ID_NEW_POSTIT = 1001;
    static const int ID_DELETE_POSTIT = 1002;
    static const int ID_LIST_POSTITS = 1003;
    static const int ID_COLOR_BUTTON = 1004;
    static const int ID_TRANSPARENCY_SLIDER = 1005;
    static const int ID_TITLE_EDIT = 1006;
    static const int ID_TEXT_EDIT = 1007;   
    static const int ID_FLOATING_NEW = 1008;
    static const int ID_FLOATING_SHOW_MAIN = 1009;
    static const int ID_BUTTON_SHOW_POSTIT = 1010;
    static const int ID_BUTTON_DELETE_POSTIT = 1011;
    static const int ID_BUTTON_HIDE_ALL = 1012;
    static const int ID_BUTTON_SHOW_ALL = 1013;
    
    // Timer IDs
    static const int ID_AUTOSAVE_TIMER = 3001;
    static const int ID_UPDATE_LIST_TIMER = 3002;
    
    // Mensajes personalizados para el system tray
    static const int WM_TRAYICON = WM_USER + 1;
    static const int ID_TRAY_ICON = 2000;
    static const int ID_TRAY_SHOW = 2001;
    static const int ID_TRAY_EXIT = 2002;
    static const int ID_TRAY_HIDE_ALL = 2003;
    static const int ID_TRAY_SHOW_ALL = 2004;   
};
