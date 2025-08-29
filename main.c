#include <Windows.h>
#include <stdio.h>
#include <commctrl.h>
#include "WebView2.h"
#include "MyHttpServer.h"

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "comctl32")
#pragma comment(lib, "user32")

// 窗口的大小
// PS：如果要让应用的逻辑像素大小受系统缩放影响，可以在 manifest.xml 中关闭dpi感知
#define WINDOW_SIZE 1280,720

// 窗口类名和窗口标题，可以按自己喜欢乱改
#define WINDOW_CLASSNAME L"com.herta.webview2Test"
#define WINDOW_TITLE L"Herta - Webview2"

// HTTP服务的静态文件根目录
#define SERVER_BASEPATH "./assets"

// HTTP服务的端口，如果端口被占用，实际使用的端口可能不同
#define SERVER_PORT 3050

// webview2 在运行的时候，会生成用户数据文件夹，可以指定其位置
#define WEBVIEW_DATA_FOLDER_PATH L"./Webview2.Data"


// 窗口句柄，通过Win32 API可以修改窗口的样式和属性
HWND window = NULL;

// HTTP服务的句柄
MyHttpServer server = NULL;

// webview 和 webviewController 可以操控Webview2控件
ICoreWebView2* webview = NULL;
ICoreWebView2Controller* webviewController = NULL;

// envHandler 和 controllerHandler 相当于带引用计数器的回调函数
ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler* envHandler = NULL;
ICoreWebView2CreateCoreWebView2ControllerCompletedHandler* controllerHandler = NULL;

// envHandler 和 controllerHandler 的引用计数器
// PS：为了少写几个函数，所以共用一个引用计数器
ULONG handlerRefCount = 0;

ULONG HandlerAddRef(IUnknown* _this) {
    return ++handlerRefCount;
}

ULONG HandlerRelease(IUnknown* _this) {
    --handlerRefCount;
    if (handlerRefCount == 0) {
        if(controllerHandler) {
            free(controllerHandler->lpVtbl);
            free(controllerHandler);
        }
        if(envHandler) {
            free(envHandler->lpVtbl);
            free(envHandler);
        }
    }
    return handlerRefCount;
}

HRESULT HandlerQueryInterface(IUnknown* _this, IID* riid, void** ppvObject) {
    *ppvObject = _this;
    HandlerAddRef(_this);
    return S_OK;
}



// 在Webview2控制器创建完成时被调用，获取 webview 和 webviewController
HRESULT ControllerHandlerInvoke(ICoreWebView2CreateCoreWebView2ControllerCompletedHandler* This, HRESULT errorCode, ICoreWebView2Controller* controller) {
    if (controller != NULL) {
        webviewController = controller;
        webviewController->lpVtbl->AddRef(webviewController);
        webviewController->lpVtbl->get_CoreWebView2(webviewController, &webview);
    }
    else return S_OK;

    // 修改Webview的配置项
    ICoreWebView2Settings* Settings;
    webview->lpVtbl->get_Settings(webview, &Settings);

    Settings->lpVtbl->put_IsScriptEnabled(Settings, TRUE);
    Settings->lpVtbl->put_AreDefaultScriptDialogsEnabled(Settings, TRUE);
    Settings->lpVtbl->put_IsWebMessageEnabled(Settings, TRUE);
    Settings->lpVtbl->put_AreDevToolsEnabled(Settings, TRUE);
    Settings->lpVtbl->put_AreDefaultContextMenusEnabled(Settings, TRUE);
    Settings->lpVtbl->put_IsStatusBarEnabled(Settings, TRUE);

    RECT bounds;
    GetClientRect(window, &bounds);
    webviewController->lpVtbl->put_Bounds(webviewController, bounds);
    
    // 打开HTTP服务对应的本地端口
    wchar_t url[MAX_PATH];
    wsprintfW(url, L"http://localhost:%d", server->port);
    webview->lpVtbl->Navigate(webview, url);

    return S_OK;
}



// 在Webview2环境创建完成时被调用，通过Webview2环境创建Webview2控制器
HRESULT EnvHandlerInvoke(ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler* This, HRESULT errorCode, ICoreWebView2Environment* env) {
    if(!env) return S_OK;
    
    controllerHandler = malloc(sizeof(ICoreWebView2CreateCoreWebView2ControllerCompletedHandler));
    controllerHandler->lpVtbl = malloc(sizeof(ICoreWebView2CreateCoreWebView2ControllerCompletedHandlerVtbl));
    controllerHandler->lpVtbl->AddRef = (void*)HandlerAddRef;
    controllerHandler->lpVtbl->Release = (void*)HandlerRelease;
    controllerHandler->lpVtbl->QueryInterface = (void*)HandlerQueryInterface;
    controllerHandler->lpVtbl->Invoke = ControllerHandlerInvoke;

    env->lpVtbl->CreateCoreWebView2Controller(env, window, controllerHandler);

    return S_OK;
}



// 窗口过程函数
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_SIZE:
        // Webview的大小自动调整为窗口大小
        if (webviewController) {
            RECT bounds;
            GetClientRect(hwnd, &bounds);
            webviewController->lpVtbl->put_Bounds(webviewController, bounds);
        }
        break;

    case WM_KEYDOWN:
        // F12键打开DevTools
        if(wParam == VK_F12 && webview) 
            webview->lpVtbl->OpenDevToolsWindow(webview);
        break;

    case WM_CLOSE:
        // 程序关闭时清理资源
        CloseMyHttpServer(server);
        PostQuitMessage(0);
        exit(0);
        break;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}



// 自定义的对话框函数
int MyMessageBoxW(HWND hWnd, UINT uType, LPCWSTR lpCaption, LPCWSTR formatText, ...) {
    wchar_t text[1024];
    va_list ap;
    va_start(ap, formatText);
    //~ vswprintf(text, sizeof(text)/sizeof(wchar_t), formatText, ap);
    _vsnwprintf(text, sizeof(text)/sizeof(wchar_t), formatText, ap);
    va_end(ap);
    
    return MessageBoxW(hWnd, text, lpCaption, uType);
}



int main() {
    // 获得当前程序实例
    HINSTANCE hInstance = GetModuleHandleW(NULL);

    // 初始化 Common Controls，从而获得更现代的窗口UI风格
    InitCommonControlsEx(&(INITCOMMONCONTROLSEX){ sizeof(INITCOMMONCONTROLSEX), ICC_WIN95_CLASSES });

    // 注册窗口类
    WNDCLASSEXW wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(32512));
    wc.hCursor = LoadCursorW(NULL, MAKEINTRESOURCEW(32512));
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = WINDOW_CLASSNAME;

    if (!RegisterClassExW(&wc)) {
        MyMessageBoxW(NULL, MB_OK | MB_ICONERROR | MB_TOPMOST, L"ERROR", L"注册窗口类时出错，错误代码：0x%08X", GetLastError());
        return -1;
    }

    // 创建窗口
    window = CreateWindowExW(0, WINDOW_CLASSNAME, WINDOW_TITLE, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_SIZE, NULL, NULL, hInstance, NULL);

    if(!window) {
        MyMessageBoxW(NULL, MB_OK | MB_ICONERROR | MB_TOPMOST, L"ERROR", L"创建窗口时出错，错误代码：0x%08X", GetLastError());
        return -1;
    }
    
    ShowWindow(window, SW_SHOWDEFAULT);
    UpdateWindow(window);

    // 在新的线程创建HTTP服务
    server = CreateMyHttpServer(SERVER_BASEPATH, SERVER_PORT);
    CreateThread(NULL, 0, (void*)StartMyHttpServer, server, 0, NULL);

    // 创建Webview2环境，当 Webview2环境创建完成后，EnvHandlerInvoke会被调用
    envHandler = malloc(sizeof(ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler));
    envHandler->lpVtbl = malloc(sizeof(ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandlerVtbl));
    envHandler->lpVtbl->AddRef = (void*)HandlerAddRef;
    envHandler->lpVtbl->Release = (void*)HandlerRelease;
    envHandler->lpVtbl->QueryInterface = (void*)HandlerQueryInterface;
    envHandler->lpVtbl->Invoke = EnvHandlerInvoke;

    HRESULT result = CreateCoreWebView2EnvironmentWithOptions(NULL, WEBVIEW_DATA_FOLDER_PATH, NULL, envHandler);
        
    if(result != S_OK) {
        // Microsoft Edge WebView2 下载网站：
        // https://developer.microsoft.com/zh-cn/microsoft-edge/webview2
        MyMessageBoxW(NULL, MB_OK | MB_ICONERROR | MB_TOPMOST, L"ERROR", L"创建Webview2时出错，错误代码: 0x%08X，请确认已安装 Microsoft Edge WebView2", result);
        return -1;
    }

    // 处理消息循环
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return (int)msg.wParam;
}