#include "../include/PostIt.h"
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <fstream>
#include <sstream>

// Para MinGW, estas librerías se enlazan con -l en lugar de #pragma
// #pragma comment(lib, "comctl32.lib")
// #pragma comment(lib, "comdlg32.lib")

PostItManager::PostItManager(HINSTANCE hInst) : mainWindow(nullptr), hInstance(hInst), 
                                                   appIcon(nullptr), backgroundBrush(nullptr), 
                                                   autoSaveTimer(0), updateListTimer(0) {
    // Inicializar controles comunes con más especificidad
    INITCOMMONCONTROLSEX icex = {};
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_BAR_CLASSES | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);
    
    // Cargar recursos de una vez
    appIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    backgroundBrush = CreateSolidBrush(RGB(240, 240, 240));
    
    // Reservar memoria para optimizar vector
    postIts.reserve(50);
}

PostItManager::~PostItManager() {
    CleanupResources();
}

bool PostItManager::Initialize() {
    // Registrar clase de ventana principal
    WNDCLASSW wc = {};
    wc.lpfnWndProc = MainWindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"PostItMainWindow";
    wc.hbrBackground = backgroundBrush;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = appIcon; // Usar icono cacheado
    
    if (!RegisterClassW(&wc)) return false;
    
    // Registrar clase para Post-it individual
    wc.lpfnWndProc = PostItWindowProc;
    wc.lpszClassName = L"PostItWindow";
    wc.hbrBackground = nullptr; // Fondo personalizado
    wc.hIcon = appIcon; // Usar el mismo icono cacheado
    
    if (!RegisterClassW(&wc)) return false;
    
    /*
    // Registrar clase para menú flotante
    wc.lpfnWndProc = FloatingMenuProc;
    wc.lpszClassName = L"FloatingMenuWindow";
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    
    if (!RegisterClassW(&wc)) return false;
    */
    
    CreateMainWindow();
    CreateSystemTrayIcon();
    LoadPostIts();
    
    return true;
}

void PostItManager::CreateMainWindow() {
    mainWindow = CreateWindowExW(
        0,
        L"PostItMainWindow",
        L"Post-it Manager",
        WS_OVERLAPPEDWINDOW, // Removido WS_VISIBLE para iniciar oculta
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 500,
        nullptr, nullptr, hInstance, this
    );
    
    // Crear controles mejorados
    CreateWindowW(L"BUTTON", L"Nuevo Post-it",
                 WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                 10, 10, 90, 30,
                 mainWindow, (HMENU)ID_NEW_POSTIT, hInstance, nullptr);
    
    CreateWindowW(L"BUTTON", L"Eliminar",
                 WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                 110, 10, 70, 30,
                 mainWindow, (HMENU)ID_DELETE_POSTIT, hInstance, nullptr);
    
    CreateWindowW(L"BUTTON", L"Mostrar",
                 WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                 190, 10, 70, 30,
                 mainWindow, (HMENU)ID_BUTTON_SHOW_POSTIT, hInstance, nullptr);
    
    CreateWindowW(L"BUTTON", L"Ocultar Todos",
                 WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                 270, 10, 90, 30,
                 mainWindow, (HMENU)ID_BUTTON_HIDE_ALL, hInstance, nullptr);
    
    CreateWindowW(L"BUTTON", L"Mostrar Todos",
                 WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                 10, 50, 90, 30,
                 mainWindow, (HMENU)ID_BUTTON_SHOW_ALL, hInstance, nullptr);
    
    CreateWindowW(L"LISTBOX", nullptr,
                 WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_NOTIFY,
                 10, 90, 360, 350,
                 mainWindow, (HMENU)ID_LIST_POSTITS, hInstance, nullptr);
    
    // Configurar auto-guardado
    autoSaveTimer = SetTimer(mainWindow, ID_AUTOSAVE_TIMER, AUTO_SAVE_INTERVAL, nullptr);
}

/*
void PostItManager::CreateFloatingMenu() {
    floatingMenu = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        L"FloatingMenuWindow",
        L"Post-it Menu",
        WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_SYSMENU,
        GetSystemMetrics(SM_CXSCREEN) - 120, 50, 100, 80,
        nullptr, nullptr, hInstance, this
    );
    
    // Botones del menú flotante
    CreateWindowW(L"BUTTON", L"Nuevo",
                 WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                 5, 5, 80, 25,
                 floatingMenu, (HMENU)ID_FLOATING_NEW, hInstance, nullptr);
    
    CreateWindowW(L"BUTTON", L"Lista",
                 WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                 5, 35, 80, 25,
                 floatingMenu, (HMENU)ID_FLOATING_SHOW_MAIN, hInstance, nullptr);
}
*/HWND PostItManager::CreatePostItWindow(PostItData& postIt) {
    // Asignar referencia al manager
    postIt.manager = this;
    
    postIt.hwnd = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TOPMOST,
        L"PostItWindow",
        postIt.title.c_str(),
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME,
        postIt.position.left, postIt.position.top,
        postIt.position.right - postIt.position.left,
        postIt.position.bottom - postIt.position.top,
        nullptr, nullptr, hInstance, &postIt
    );
    
    if (postIt.hwnd) {
        // Aplicar transparencia
        SetLayeredWindowAttributes(postIt.hwnd, 0, postIt.transparency, LWA_ALPHA);
        
        // Mostrar la ventana explícitamente si debe ser visible
        if (postIt.isVisible) {
            ShowWindow(postIt.hwnd, SW_SHOW);
            UpdateWindow(postIt.hwnd);
        }
    }
    
    // Crear controles dentro del Post-it
    CreateWindowW(L"STATIC", L"Título:",
                 WS_VISIBLE | WS_CHILD,
                 5, 5, 40, 20,
                 postIt.hwnd, nullptr, hInstance, nullptr);
    
    CreateWindowW(L"EDIT", postIt.title.c_str(),
                 WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
                 50, 5, 120, 20,
                 postIt.hwnd, (HMENU)ID_TITLE_EDIT, hInstance, nullptr);
    
    CreateWindowW(L"EDIT", postIt.text.c_str(),
                 WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_WANTRETURN | WS_VSCROLL,
                 5, 30, 165, 80,
                 postIt.hwnd, (HMENU)ID_TEXT_EDIT, hInstance, nullptr);
    
    CreateWindowW(L"BUTTON", L"Color",
                 WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                 5, 115, 50, 25,
                 postIt.hwnd, (HMENU)ID_COLOR_BUTTON, hInstance, nullptr);
    
    // Etiqueta para transparencia
    CreateWindowW(L"STATIC", L"Opacidad:",
                 WS_VISIBLE | WS_CHILD,
                 60, 95, 60, 15,
                 postIt.hwnd, nullptr, hInstance, nullptr);
    
    HWND slider = CreateWindowW(TRACKBAR_CLASSW, nullptr,
                 WS_VISIBLE | WS_CHILD | TBS_HORZ | TBS_TOOLTIPS,
                 60, 115, 110, 25,
                 postIt.hwnd, (HMENU)ID_TRANSPARENCY_SLIDER, hInstance, nullptr);
    
    // Configurar el slider de transparencia (50 = más transparente, 255 = opaco)
    SendMessage(slider, TBM_SETRANGE, TRUE, MAKELPARAM(50, 255));
    SendMessage(slider, TBM_SETPOS, TRUE, postIt.transparency);
    SendMessage(slider, TBM_SETTICFREQ, 51, 0); // Marcas cada 51 unidades
    SendMessage(slider, TBM_SETPAGESIZE, 0, 20); // Tamaño de página para navegación
    
    return postIt.hwnd;
}

void PostItManager::CreateNewPostIt() {
    PostItData newPostIt;
    newPostIt.title = L"Nuevo Post-it";
    newPostIt.text = L"Ingresa tu texto aquí...";
    newPostIt.isVisible = true; // Nuevo Post-it debe ser visible
    newPostIt.manager = this; // IMPORTANTE: Asignar el manager
    
    postIts.push_back(newPostIt);
    HWND newWindow = CreatePostItWindow(postIts.back());
    
    // Asegurar que la ventana se muestra
    if (newWindow) {
        ShowWindow(newWindow, SW_SHOW);
        SetForegroundWindow(newWindow);
        BringWindowToTop(newWindow);
    }
    
    UpdateMainWindowList();
    // Guardar el nuevo Post-it
    SavePostIts();
}

void PostItManager::UpdateMainWindowList() {
    HWND listBox = GetDlgItem(mainWindow, ID_LIST_POSTITS);
    if (!listBox) return;
    
    // Guardar selección actual
    int currentSelection = (int)SendMessage(listBox, LB_GETCURSEL, 0, 0);
    std::wstring selectedTitle;
    
    if (currentSelection != LB_ERR && currentSelection < (int)postIts.size()) {
        selectedTitle = postIts[currentSelection].title;
    }
    
    // Limpiar y repoblar la lista
    SendMessage(listBox, LB_RESETCONTENT, 0, 0);
    
    for (size_t i = 0; i < postIts.size(); ++i) {
        const auto& postIt = postIts[i];
        // Agregar indicador visual de visibilidad
        std::wstring displayText = postIt.title;
        if (!postIt.isVisible) {
            displayText += L" (oculto)";
        }
        SendMessage(listBox, LB_ADDSTRING, 0, (LPARAM)displayText.c_str());
    }
    
    // Restaurar selección si es posible
    if (!selectedTitle.empty()) {
        for (size_t i = 0; i < postIts.size(); ++i) {
            if (postIts[i].title == selectedTitle) {
                SendMessage(listBox, LB_SETCURSEL, i, 0);
                break;
            }
        }
    }
}

LRESULT CALLBACK PostItManager::MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    PostItManager* manager = nullptr;
    
    if (uMsg == WM_NCCREATE) {
        CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
        manager = (PostItManager*)cs->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)manager);
    } else {
        manager = (PostItManager*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }
    
    if (manager) {
        switch (uMsg) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
            case ID_NEW_POSTIT:
                manager->CreateNewPostIt();
                break;
            case ID_DELETE_POSTIT: {
                HWND listBox = GetDlgItem(hwnd, ID_LIST_POSTITS);
                int selected = (int)SendMessage(listBox, LB_GETCURSEL, 0, 0);
                if (selected != LB_ERR && selected < (int)manager->postIts.size()) {
                    if (manager->postIts[selected].hwnd) {
                        DestroyWindow(manager->postIts[selected].hwnd);
                    }
                    manager->postIts.erase(manager->postIts.begin() + selected);
                    manager->UpdateMainWindowList();
                    // Guardar cambios después de eliminar
                    manager->SavePostIts();
                }
                break;
            }
            case ID_LIST_POSTITS:
                if (HIWORD(wParam) == LBN_DBLCLK) {
                    // Doble clic en la lista - mostrar Post-it seleccionado
                    HWND listBox = GetDlgItem(hwnd, ID_LIST_POSTITS);
                    int selected = (int)SendMessage(listBox, LB_GETCURSEL, 0, 0);
                    
                    if (selected != LB_ERR && selected >= 0 && selected < (int)manager->postIts.size()) {
                        PostItData& postIt = manager->postIts[selected];
                        
                        // Si la ventana no existe, recrearla
                        if (!postIt.hwnd || !IsWindow(postIt.hwnd)) {
                            postIt.hwnd = manager->CreatePostItWindow(postIt);
                        }
                        
                        // Mostrar la ventana
                        if (postIt.hwnd && IsWindow(postIt.hwnd)) {
                            ShowWindow(postIt.hwnd, SW_RESTORE);
                            ShowWindow(postIt.hwnd, SW_SHOW);
                            SetForegroundWindow(postIt.hwnd);
                            BringWindowToTop(postIt.hwnd);
                            SetActiveWindow(postIt.hwnd);
                            postIt.isVisible = true;
                        }
                    }
                }
                break;
            case ID_TRAY_SHOW:
                ShowWindow(hwnd, SW_RESTORE);
                SetForegroundWindow(hwnd);
                break;
            case ID_TRAY_SHOW_ALL:
                manager->ShowAllPostIts();
                break;
            case ID_TRAY_HIDE_ALL:
                manager->HideAllPostIts();
                break;
            case ID_TRAY_EXIT:
                // Cerrar todas las ventanas de Post-it antes de salir
                for (auto& postIt : manager->postIts) {
                    if (postIt.hwnd) {
                        DestroyWindow(postIt.hwnd);
                    }
                }
                // Cerrar realmente la aplicación
                if (manager) {
                    manager->SavePostIts();
                    manager->RemoveSystemTrayIcon();
                }
                DestroyWindow(hwnd);
                PostQuitMessage(0);
                break;
            case ID_BUTTON_SHOW_POSTIT: // Botón "Mostrar"
                {
                    HWND listBox = GetDlgItem(hwnd, ID_LIST_POSTITS);
                    int selected = (int)SendMessage(listBox, LB_GETCURSEL, 0, 0);
                    
                    if (selected != LB_ERR && selected >= 0 && selected < (int)manager->postIts.size()) {
                        PostItData& postIt = manager->postIts[selected];
                        
                        // Si la ventana no existe, recrearla
                        if (!postIt.hwnd || !IsWindow(postIt.hwnd)) {
                            postIt.hwnd = manager->CreatePostItWindow(postIt);
                        }
                        
                        // Mostrar la ventana
                        if (postIt.hwnd && IsWindow(postIt.hwnd)) {
                            ShowWindow(postIt.hwnd, SW_RESTORE);
                            ShowWindow(postIt.hwnd, SW_SHOW);
                            SetForegroundWindow(postIt.hwnd);
                            BringWindowToTop(postIt.hwnd);
                            SetActiveWindow(postIt.hwnd);
                            postIt.isVisible = true;
                            
                            // Guardar el estado
                            if (manager) {
                                manager->SavePostIts();
                            }
                        }
                    }
                }
                break;
            case ID_BUTTON_HIDE_ALL: // Botón "Ocultar Todos"
                manager->HideAllPostIts();
                break;
            case ID_BUTTON_SHOW_ALL: // Botón "Mostrar Todos"
                manager->ShowAllPostIts();
                break;
            }
            break;
        case WM_TRAYICON:
            switch (lParam) {
            case WM_LBUTTONDBLCLK:
                // Doble clic en el ícono de la bandeja - mostrar ventana principal
                ShowWindow(hwnd, SW_RESTORE);
                SetForegroundWindow(hwnd);
                break;
            case WM_RBUTTONDOWN: {
                // Clic derecho en el ícono de la bandeja - mostrar menú contextual
                POINT pt;
                GetCursorPos(&pt);
                manager->ShowTrayContextMenu(hwnd, pt);
                break;
            }
            }
            break;
        case WM_SYSCOMMAND:
            if (wParam == SC_MINIMIZE) {
                // Cuando se minimiza, ocultar ventana en lugar de minimizar
                ShowWindow(hwnd, SW_HIDE);
                return 0;
            }
            break;
        case WM_TIMER:
            if (wParam == ID_AUTOSAVE_TIMER && manager) {
                manager->AutoSave();
            } else if (wParam == ID_UPDATE_LIST_TIMER && manager) {
                // Actualizar la lista y detener el timer
                manager->UpdateMainWindowList();
                KillTimer(hwnd, ID_UPDATE_LIST_TIMER);
                manager->updateListTimer = 0;
                // También guardar los cambios
                manager->SavePostIts();
            }
            break;
        case WM_CLOSE:
            // Ocultar ventana en lugar de cerrarla
            ShowWindow(hwnd, SW_HIDE);
            return 0;
        case WM_DESTROY:
            // Guardar antes de cerrar la aplicación
            if (manager) {
                manager->SavePostIts();
                manager->RemoveSystemTrayIcon();
            }
            PostQuitMessage(0);
            break;
        }
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK PostItManager::PostItWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    PostItData* postIt = nullptr;
    
    if (uMsg == WM_NCCREATE) {
        CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
        postIt = (PostItData*)cs->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)postIt);
    } else {
        postIt = (PostItData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }
    
    switch (uMsg) {
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORSTATIC: {
        if (postIt) {
            HDC hdc = (HDC)wParam;
            HWND hControl = (HWND)lParam;
            
            // Verificar si es uno de nuestros controles de texto
            int controlId = GetDlgCtrlID(hControl);
            if (controlId == ID_TITLE_EDIT || controlId == ID_TEXT_EDIT) {
                // Calcular un color de texto que contraste con el fondo
                COLORREF bgColor = postIt->color;
                int r = GetRValue(bgColor);
                int g = GetGValue(bgColor);
                int b = GetBValue(bgColor);
                
                // Calcular luminancia para decidir si usar texto negro o blanco
                double luminance = (0.299 * r + 0.587 * g + 0.114 * b) / 255.0;
                COLORREF textColor = luminance > 0.5 ? RGB(0, 0, 0) : RGB(255, 255, 255);
                
                SetTextColor(hdc, textColor);
                SetBkColor(hdc, bgColor);
                
                // Crear brush con el color del Post-it
                HBRUSH hBrush = CreateSolidBrush(bgColor);
                return (LRESULT)hBrush;
            }
            else {
                // Para otros controles (labels), usar fondo con el color del Post-it
                COLORREF bgColor = postIt->color;
                int r = GetRValue(bgColor);
                int g = GetGValue(bgColor);
                int b = GetBValue(bgColor);
                
                // Calcular luminancia para el color del texto
                double luminance = (0.299 * r + 0.587 * g + 0.114 * b) / 255.0;
                COLORREF textColor = luminance > 0.5 ? RGB(0, 0, 0) : RGB(255, 255, 255);
                
                SetTextColor(hdc, textColor);
                SetBkColor(hdc, bgColor);
                
                HBRUSH hBrush = CreateSolidBrush(bgColor);
                return (LRESULT)hBrush;
            }
        }
        break;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        
        RECT rect;
        GetClientRect(hwnd, &rect);
        
        HBRUSH brush = CreateSolidBrush(postIt ? postIt->color : RGB(255, 255, 0));
        FillRect(hdc, &rect, brush);
        DeleteObject(brush);
        
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_COMMAND:
        if (postIt) {
            switch (LOWORD(wParam)) {
            case ID_COLOR_BUTTON: {
                // Usar CHOOSECOLOR estándar para mejor compatibilidad
                CHOOSECOLOR cc = {};
                COLORREF customColors[16] = {};
                
                // Inicializar colores personalizados con algunos predeterminados
                customColors[0] = RGB(255, 255, 0);   // Amarillo
                customColors[1] = RGB(255, 192, 203); // Rosa
                customColors[2] = RGB(173, 216, 230); // Azul claro
                customColors[3] = RGB(144, 238, 144); // Verde claro
                customColors[4] = RGB(255, 165, 0);   // Naranja
                customColors[5] = RGB(221, 160, 221); // Ciruela claro
                
                cc.lStructSize = sizeof(CHOOSECOLOR);
                cc.hwndOwner = hwnd;
                cc.rgbResult = postIt->color;
                cc.lpCustColors = customColors;
                cc.Flags = CC_FULLOPEN | CC_RGBINIT | CC_ANYCOLOR;
                
                if (ChooseColor(&cc)) {
                    postIt->color = cc.rgbResult;
                    // Repintar inmediatamente toda la ventana y sus controles
                    InvalidateRect(hwnd, nullptr, TRUE);
                    // Forzar que todos los controles hijos se redibujen también
                    HWND hChild = GetWindow(hwnd, GW_CHILD);
                    while (hChild) {
                        InvalidateRect(hChild, nullptr, TRUE);
                        UpdateWindow(hChild);
                        hChild = GetWindow(hChild, GW_HWNDNEXT);
                    }
                    UpdateWindow(hwnd);
                    // Guardar cambios
                    if (postIt->manager) {
                        postIt->manager->SavePostIts();
                    }
                }
                break;
            }
            case ID_TITLE_EDIT:
                if (HIWORD(wParam) == EN_CHANGE) {
                    wchar_t buffer[256];
                    GetWindowTextW((HWND)lParam, buffer, 256);
                    postIt->title = buffer;
                    SetWindowTextW(hwnd, buffer);
                    
                    // Programar actualización de la lista con delay
                    if (postIt->manager) {
                        // Cancelar timer anterior si existe
                        if (postIt->manager->updateListTimer) {
                            KillTimer(postIt->manager->mainWindow, ID_UPDATE_LIST_TIMER);
                        }
                        // Programar nueva actualización
                        postIt->manager->updateListTimer = SetTimer(
                            postIt->manager->mainWindow, 
                            ID_UPDATE_LIST_TIMER, 
                            UPDATE_LIST_DELAY, 
                            nullptr
                        );
                    }
                }
                break;
            case ID_TEXT_EDIT:
                if (HIWORD(wParam) == EN_CHANGE) {
                    int len = GetWindowTextLengthW((HWND)lParam);
                    wchar_t* buffer = new wchar_t[len + 1];
                    GetWindowTextW((HWND)lParam, buffer, len + 1);
                    postIt->text = buffer;
                    delete[] buffer;
                    // Guardar cambios
                    if (postIt->manager) {
                        postIt->manager->SavePostIts();
                    }
                }
                break;
            }
        }
        break;
    case WM_HSCROLL:
    case WM_VSCROLL:
        if (postIt && (HWND)lParam == GetDlgItem(hwnd, ID_TRANSPARENCY_SLIDER)) {
            int pos = (int)SendMessage((HWND)lParam, TBM_GETPOS, 0, 0);
            postIt->transparency = pos;
            SetLayeredWindowAttributes(hwnd, 0, pos, LWA_ALPHA);
            
            // Actualizar título de ventana con info de opacidad para debugging
            wchar_t title[256];
            swprintf_s(title, 256, L"%s (Opacidad: %d%%)", postIt->title.c_str(), (pos * 100) / 255);
            SetWindowTextW(hwnd, title);
            
            // Guardar cambios
            if (postIt->manager) {
                postIt->manager->SavePostIts();
            }
        }
        break;
    case WM_MOVING:
    case WM_SIZING:
        if (postIt) {
            RECT* rect = (RECT*)lParam;
            postIt->position = *rect;
            // Guardar cambios de posición
            if (postIt->manager) {
                postIt->manager->SavePostIts();
            }
        }
        break;
    case WM_DESTROY:
        // Aquí se pueden limpiar recursos específicos si es necesario
        break;
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

/*
LRESULT CALLBACK PostItManager::FloatingMenuProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    PostItManager* manager = nullptr;
    
    if (uMsg == WM_NCCREATE) {
        CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
        manager = (PostItManager*)cs->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)manager);
    } else {
        manager = (PostItManager*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }
    
    if (manager) {
        switch (uMsg) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
            case ID_FLOATING_NEW:
                manager->CreateNewPostIt();
                break;
            case ID_FLOATING_SHOW_MAIN:
                ShowWindow(manager->mainWindow, SW_RESTORE);
                SetForegroundWindow(manager->mainWindow);
                break;
            }
            break;
        }
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
*/

void PostItManager::Run() {
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void PostItManager::SavePostIts() {
    // Usar archivos binarios para evitar problemas con Unicode
    std::ofstream file("postits.dat", std::ios::binary);
    if (file.is_open()) {
        // Guardar el número de post-its
        size_t count = postIts.size();
        file.write(reinterpret_cast<const char*>(&count), sizeof(count));
        
        for (const auto& postIt : postIts) {
            // Guardar título
            size_t titleLen = postIt.title.length();
            file.write(reinterpret_cast<const char*>(&titleLen), sizeof(titleLen));
            file.write(reinterpret_cast<const char*>(postIt.title.c_str()), titleLen * sizeof(wchar_t));
            
            // Guardar texto
            size_t textLen = postIt.text.length();
            file.write(reinterpret_cast<const char*>(&textLen), sizeof(textLen));
            file.write(reinterpret_cast<const char*>(postIt.text.c_str()), textLen * sizeof(wchar_t));
            
            // Guardar otros datos
            file.write(reinterpret_cast<const char*>(&postIt.color), sizeof(postIt.color));
            file.write(reinterpret_cast<const char*>(&postIt.transparency), sizeof(postIt.transparency));
            file.write(reinterpret_cast<const char*>(&postIt.position), sizeof(postIt.position));
            file.write(reinterpret_cast<const char*>(&postIt.isVisible), sizeof(postIt.isVisible));
        }
        file.close();
    }
}

void PostItManager::LoadPostIts() {
    std::ifstream file("postits.dat", std::ios::binary);
    if (file.is_open()) {
        // Leer el número de post-its
        size_t count;
        file.read(reinterpret_cast<char*>(&count), sizeof(count));
        
        for (size_t i = 0; i < count; ++i) {
            PostItData postIt;
            
            // Leer título
            size_t titleLen;
            file.read(reinterpret_cast<char*>(&titleLen), sizeof(titleLen));
            postIt.title.resize(titleLen);
            file.read(reinterpret_cast<char*>(&postIt.title[0]), titleLen * sizeof(wchar_t));
            
            // Leer texto
            size_t textLen;
            file.read(reinterpret_cast<char*>(&textLen), sizeof(textLen));
            postIt.text.resize(textLen);
            file.read(reinterpret_cast<char*>(&postIt.text[0]), textLen * sizeof(wchar_t));
            
            // Leer otros datos
            file.read(reinterpret_cast<char*>(&postIt.color), sizeof(postIt.color));
            file.read(reinterpret_cast<char*>(&postIt.transparency), sizeof(postIt.transparency));
            file.read(reinterpret_cast<char*>(&postIt.position), sizeof(postIt.position));
            file.read(reinterpret_cast<char*>(&postIt.isVisible), sizeof(postIt.isVisible));
            
            // IMPORTANTE: Asignar el manager al Post-it cargado
            postIt.manager = this;
            
            postIts.push_back(postIt);
            CreatePostItWindow(postIts.back());
        }
        
        UpdateMainWindowList();
        file.close();
    }
}

void PostItManager::CreateSystemTrayIcon() {
    NOTIFYICONDATAW nid = {};
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = mainWindow;
    nid.uID = ID_TRAY_ICON;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = appIcon ? appIcon : LoadIcon(hInstance, MAKEINTRESOURCE(1)); // Usar icono cacheado
    wcscpy_s(nid.szTip, 128, L"Post-it Manager - Mejorado con auto-guardado");
    
    Shell_NotifyIconW(NIM_ADD, &nid);
}

void PostItManager::RemoveSystemTrayIcon() {
    NOTIFYICONDATAW nid = {};
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = mainWindow;
    nid.uID = ID_TRAY_ICON;
    
    Shell_NotifyIconW(NIM_DELETE, &nid);
}

void PostItManager::ShowTrayContextMenu(HWND hwnd, POINT pt) {
    HMENU hMenu = CreatePopupMenu();
    if (hMenu) {
        AppendMenuW(hMenu, MF_STRING, ID_TRAY_SHOW, L"Mostrar Ventana Principal");
        AppendMenuW(hMenu, MF_STRING, ID_NEW_POSTIT, L"Nuevo Post-it");
        AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
        AppendMenuW(hMenu, MF_STRING, ID_TRAY_SHOW_ALL, L"Mostrar Todos los Post-its");
        AppendMenuW(hMenu, MF_STRING, ID_TRAY_HIDE_ALL, L"Ocultar Todos los Post-its");
        AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
        AppendMenuW(hMenu, MF_STRING, ID_TRAY_EXIT, L"Salir");
        
        // Hacer que la ventana sea la ventana activa antes de mostrar el menú
        SetForegroundWindow(hwnd);
        
        TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, nullptr);
        
        // Debe ser llamado después de TrackPopupMenu
        PostMessage(hwnd, WM_NULL, 0, 0);
        
        DestroyMenu(hMenu);
    }
}

// ===== MÉTODOS DE OPTIMIZACIÓN Y MEJORAS =====

void PostItManager::CleanupResources() {
    // Detener timers
    if (autoSaveTimer && mainWindow) {
        KillTimer(mainWindow, ID_AUTOSAVE_TIMER);
        autoSaveTimer = 0;
    }
    
    if (updateListTimer && mainWindow) {
        KillTimer(mainWindow, ID_UPDATE_LIST_TIMER);
        updateListTimer = 0;
    }
    
    // Guardar antes de limpiar
    SavePostIts();
    
    // Cerrar todas las ventanas de Post-it
    for (auto& postIt : postIts) {
        if (postIt.hwnd && IsWindow(postIt.hwnd)) {
            DestroyWindow(postIt.hwnd);
        }
    }
    
    // Limpiar recursos cacheados
    if (appIcon) {
        DestroyIcon(appIcon);
        appIcon = nullptr;
    }
    
    if (backgroundBrush) {
        DeleteObject(backgroundBrush);
        backgroundBrush = nullptr;
    }
    
    RemoveSystemTrayIcon();
    postIts.clear();
}

void PostItManager::AutoSave() {
    SavePostIts();
}

bool PostItManager::ValidatePostItIndex(int index) {
    return (index >= 0 && index < static_cast<int>(postIts.size()));
}

void PostItManager::DeletePostIt(int index) {
    if (!ValidatePostItIndex(index)) return;
    
    // Cerrar ventana si existe
    if (postIts[index].hwnd && IsWindow(postIts[index].hwnd)) {
        DestroyWindow(postIts[index].hwnd);
    }
    
    // Eliminar del vector
    postIts.erase(postIts.begin() + index);
    
    // Actualizar lista y guardar
    UpdateMainWindowList();
    SavePostIts();
}

void PostItManager::HideAllPostIts() {
    for (auto& postIt : postIts) {
        if (postIt.hwnd && IsWindow(postIt.hwnd)) {
            ShowWindow(postIt.hwnd, SW_HIDE);
            postIt.isVisible = false;
        }
    }
    SavePostIts();
}

void PostItManager::ShowAllPostIts() {
    for (auto& postIt : postIts) {
        // Recrear ventana si es necesario
        if (!postIt.hwnd || !IsWindow(postIt.hwnd)) {
            postIt.hwnd = CreatePostItWindow(postIt);
        }
        
        if (postIt.hwnd && IsWindow(postIt.hwnd)) {
            ShowWindow(postIt.hwnd, SW_SHOW);
            SetWindowPos(postIt.hwnd, HWND_TOP, 0, 0, 0, 0, 
                        SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
            postIt.isVisible = true;
        }
    }
    SavePostIts();
}

void PostItManager::MinimizeToTray() {
    if (mainWindow) {
        ShowWindow(mainWindow, SW_HIDE);
    }
}

void PostItManager::OptimizeLayout() {
    // Reorganizar Post-its para evitar solapamiento
    int offsetX = 0, offsetY = 0;
    const int SPACING = 20;
    
    for (size_t i = 0; i < postIts.size(); ++i) {
        postIts[i].position.left = 100 + offsetX;
        postIts[i].position.top = 100 + offsetY;
        postIts[i].position.right = postIts[i].position.left + 220;
        postIts[i].position.bottom = postIts[i].position.top + 170;
        
        // Actualizar posición de la ventana si existe
        if (postIts[i].hwnd && IsWindow(postIts[i].hwnd)) {
            SetWindowPos(postIts[i].hwnd, nullptr,
                        postIts[i].position.left, postIts[i].position.top,
                        220, 170, SWP_NOZORDER | SWP_NOACTIVATE);
        }
        
        offsetX += SPACING;
        offsetY += SPACING;
        
        // Reiniciar offset si sale de pantalla
        if (offsetX > 800 || offsetY > 600) {
            offsetX = 0;
            offsetY = 0;
        }
    }
    
    SavePostIts();
}
