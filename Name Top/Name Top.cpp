#include "framework.h"
#include "Name Top.h"
#include <string>

#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                     // 当前实例
WCHAR szTitle[MAX_LOADSTRING];       // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING]; // 主窗口类名
NOTIFYICONDATA nid;                  // 托盘图标数据

// 学生名字和字体设置
std::wstring studentName = L""; // 学生的名字
const int fontSize       = 150; // 字体大小
const int marginTop      = 0;   // 窗口顶部距离
const int marginRight    = 120; // 窗口右边距离

// 此代码模块中包含的函数的前向声明:
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
void UpdateName(HWND hWnd);

int APIENTRY
wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_NAMETOP, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_NAMETOP));

    MSG msg;

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style         = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc   = WndProc;
    wcex.cbClsExtra    = 0;
    wcex.cbWndExtra    = 0;
    wcex.hInstance     = hInstance;
    wcex.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NAMETOP));
    wcex.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = nullptr; // 背景透明
    wcex.lpszMenuName  = nullptr; // 去掉菜单栏
    wcex.lpszMenuName  = MAKEINTRESOURCEW(IDC_NAMETOP);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm       = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // 将实例句柄存储在全局变量中

    // 创建窗口
    HWND hWnd = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW, // 窗口样式
        szWindowClass,                                                        // 窗口类名
        szTitle,                                                              // 窗口标题
        WS_POPUP,                                                             // 无边框
        CW_USEDEFAULT,
        0,
        CW_USEDEFAULT,
        0, // 初始位置和大小
        nullptr,
        nullptr,
        hInstance,
        nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    // 计算窗口位置和大小
    RECT desktopRect;
    GetWindowRect(GetDesktopWindow(), &desktopRect);
    int windowWidth  = 500; // 窗口宽度
    int windowHeight = 200; // 窗口高度
    int x            = desktopRect.right - windowWidth - marginRight;
    int y            = marginTop;

    // 设置窗口位置和大小
    SetWindowPos(hWnd, nullptr, x, y, windowWidth, windowHeight, SWP_SHOWWINDOW);

    // 设置窗口透明
    SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

    // 提示用户输入名字
    UpdateName(hWnd);

    // 初始化托盘图标
    nid.cbSize           = sizeof(NOTIFYICONDATA);
    nid.hWnd             = hWnd;
    nid.uID              = 1; // 图标 ID
    nid.uFlags           = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_USER + 1;                                       // 自定义消息
    nid.hIcon            = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NAMETOP)); // 图标资源
    wcscpy_s(nid.szTip, L"Name Top");                                         // 鼠标悬停提示

    // 添加托盘图标
    Shell_NotifyIcon(NIM_ADD, &nid);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        // 第一步：用颜色键颜色填充背景（关键步骤）
        HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0)); // 创建黑色画刷
        FillRect(hdc, &ps.rcPaint, hBrush);             // 填充整个客户区
        DeleteObject(hBrush);                           // 删除画刷

        // 第二步：创建字体（加粗字体）
        HFONT hFont = CreateFont(
            fontSize,
            0,
            0,
            0,
            FW_BOLD, // 加粗字体
            FALSE,
            FALSE,
            FALSE,
            DEFAULT_CHARSET,
            OUT_OUTLINE_PRECIS,
            CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY,
            VARIABLE_PITCH,
            TEXT("微软雅黑"));
        SelectObject(hdc, hFont);

        // 第三步：设置文字颜色
        SetTextColor(hdc, RGB(255, 0, 0)); // 红色文字

        // 第四步：设置文本背景透明
        SetBkMode(hdc, TRANSPARENT); // 关键：设置文本背景透明

        // 第五步：绘制文本
        RECT rect;
        GetClientRect(hWnd, &rect);
        DrawText(hdc, studentName.c_str(), -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        // 清理资源
        DeleteObject(hFont);
        EndPaint(hWnd, &ps);
        break;
    }
    case WM_USER + 1: { // 自定义消息
        switch (LOWORD(lParam))
        {
        case WM_RBUTTONUP: // 右键点击
        {
            // 显示弹出菜单
            HMENU hMenu = CreatePopupMenu();
            AppendMenu(hMenu, MF_STRING, 1001, L"修改名字"); // 添加修改名字选项
            AppendMenu(hMenu, MF_STRING, IDM_EXIT, L"退出");

            POINT pt;
            GetCursorPos(&pt);
            SetForegroundWindow(hWnd); // 确保菜单显示在最前面
            TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
            DestroyMenu(hMenu);
            break;
        }
        case WM_LBUTTONDBLCLK: // 双击左键
        {
            // 恢复窗口显示
            ShowWindow(hWnd, SW_SHOW);
            break;
        }
        }
        break;
    }
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case 1001: // "修改名字"命令
            UpdateName(hWnd);
            break;
        case IDM_EXIT: // "退出"命令
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    }
    case WM_DESTROY:
        // 移除托盘图标
        Shell_NotifyIcon(NIM_DELETE, &nid);
        // 终止消息循环
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void UpdateName(HWND hWnd)
{
    WCHAR name[MAX_LOADSTRING] = {0};
    wcscpy_s(name, studentName.c_str());

    WCHAR* pName = (WCHAR*)GlobalAlloc(GPTR, MAX_LOADSTRING * sizeof(WCHAR));
    if (pName)
    {
        wcscpy_s(pName, MAX_LOADSTRING, name);

        if (DialogBoxParam(
                hInst,
                MAKEINTRESOURCE(IDD_DIALOG1),
                hWnd,
                [](HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) -> INT_PTR {
                    switch (message)
                    {
                    case WM_INITDIALOG:
                        // 保存传入的指针到对话框的用户数据
                        SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
                        SetDlgItemText(hDlg, IDC_EDIT1, (WCHAR*)lParam);
                        return (INT_PTR)TRUE;

                    case WM_COMMAND:
                        if (LOWORD(wParam) == IDOK)
                        {
                            // 从用户数据中获取指针
                            WCHAR* pName = (WCHAR*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
                            GetDlgItemText(hDlg, IDC_EDIT1, pName, MAX_LOADSTRING);
                            studentName = pName;
                            InvalidateRect(GetParent(hDlg), NULL, TRUE);
                            EndDialog(hDlg, IDOK);
                            return (INT_PTR)TRUE;
                        }
                        else if (LOWORD(wParam) == IDCANCEL)
                        {
                            EndDialog(hDlg, IDCANCEL);
                            return (INT_PTR)TRUE;
                        }
                        break;
                    }
                    return (INT_PTR)FALSE;
                },
                (LPARAM)pName)
            == IDOK)
        {
            // 用户确认修改
        }

        GlobalFree(pName);
    }
}