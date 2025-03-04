#include "framework.h"
#include "Name Top.h"

#include <string>

#include <windows.h>
#include <comdef.h>
#include <commctrl.h>
#include <gdiplus.h>
#include <shellapi.h>

#pragma comment(lib, "gdiplus.lib")

#define MAX_LOADSTRING 100

// 全局变量声明
//--------------------------------------------------
HINSTANCE hInst;                     // 当前实例句柄
WCHAR szTitle[MAX_LOADSTRING];       // 标题栏文本缓冲区
WCHAR szWindowClass[MAX_LOADSTRING]; // 窗口类名缓冲区
BOOL g_bDialogActive = FALSE;        // 对话框激活状态标记（防止置顶干扰）
NOTIFYICONDATA nid;                  // 系统托盘图标数据结构
int g_alpha         = 255;           // 窗口透明度（255=不透明）
const int MIN_ALPHA = 204;           // 最小透明度（204=80%）

// 显示相关配置
std::wstring studentName = L""; // 显示的学生姓名
const int fontSize       = 150; // 字体大小（像素）
const int marginTop      = 0;   // 窗口顶部边距
const int marginRight    = 120; // 窗口右侧边距

// 函数前置声明
//--------------------------------------------------
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
void UpdateName(HWND hWnd);

// 程序入口点
//--------------------------------------------------
int APIENTRY
wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // 初始化全局字符串资源
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_NAMETOP, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 创建窗口实例
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    // 加载快捷键表
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_NAMETOP));

    // 主消息循环
    MSG msg;
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

// 窗口类注册函数
//--------------------------------------------------
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    // 配置窗口类属性
    wcex.style         = CS_HREDRAW | CS_VREDRAW;                              // 窗口变化时重绘
    wcex.lpfnWndProc   = WndProc;                                              // 窗口过程函数
    wcex.cbClsExtra    = 0;                                                    // 无额外类内存
    wcex.cbWndExtra    = 0;                                                    // 无额外窗口内存
    wcex.hInstance     = hInstance;                                            // 实例句柄
    wcex.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NAMETOP));    // 程序图标
    wcex.hCursor       = LoadCursor(nullptr, IDC_ARROW);                       // 箭头光标
    wcex.hbrBackground = nullptr;                                              // 透明背景（需配合WS_EX_LAYERED）
    wcex.lpszMenuName  = nullptr;                                              // 无菜单
    wcex.lpszClassName = szWindowClass;                                        // 窗口类名
    wcex.hIconSm       = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL)); // 小图标

    return RegisterClassExW(&wcex);
}

// 窗口实例初始化
//--------------------------------------------------
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // 存储实例句柄到全局变量

    // 创建主窗口（初始位置和大小将在后续设置）
    HWND hWnd = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW, // 扩展样式：分层窗口+透明+不显示在任务栏
        szWindowClass,                                        // 窗口类名
        szTitle,                                              // 窗口标题
        WS_POPUP,                                             // 无边框窗口
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

    // 计算窗口位置（屏幕右上角）
    RECT desktopRect;
    GetWindowRect(GetDesktopWindow(), &desktopRect);
    const int windowWidth  = 500; // 窗口宽度
    const int windowHeight = 200; // 窗口高度
    const int x            = desktopRect.right - windowWidth - marginRight;
    const int y            = marginTop;

    // 设置窗口尺寸和位置
    SetWindowPos(hWnd, nullptr, x, y, windowWidth, windowHeight, SWP_SHOWWINDOW);

    // 配置窗口透明属性（使用颜色键透明）
    SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

    // 首次输入姓名
    UpdateName(hWnd);

    // 初始化系统托盘图标
    ZeroMemory(&nid, sizeof(NOTIFYICONDATA));
    nid.cbSize           = sizeof(NOTIFYICONDATA);
    nid.hWnd             = hWnd;
    nid.uID              = 1;                                                 // 托盘图标ID
    nid.uFlags           = NIF_ICON | NIF_MESSAGE | NIF_TIP;                  // 支持的属性
    nid.uCallbackMessage = WM_USER + 1;                                       // 自定义消息编号
    nid.hIcon            = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NAMETOP)); // 托盘图标
    wcscpy_s(nid.szTip, L"Name Top");                                         // 悬停提示文本

    // 添加托盘图标
    Shell_NotifyIcon(NIM_ADD, &nid);

    // 显示窗口
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

// 主窗口消息处理
//--------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    // 窗口绘制消息
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        /* 绘制步骤说明：
           1. 用颜色键颜色填充背景（实现透明效果）
           2. 创建加粗字体
           3. 设置红色文字和透明背景
           4. 居中绘制文本
        */

        // 步骤1：填充黑色背景（将被透明处理）
        HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
        FillRect(hdc, &ps.rcPaint, hBrush);
        DeleteObject(hBrush);

        // 步骤2：创建加粗字体
        HFONT hFont = CreateFont(
            fontSize,           // 字高
            0,                  // 字宽（0=自动匹配）
            0,                  // 文本倾斜度
            0,                  // 字体倾斜度
            FW_BOLD,            // 加粗
            FALSE,              // 斜体
            FALSE,              // 下划线
            FALSE,              // 删除线
            DEFAULT_CHARSET,    // 字符集
            OUT_OUTLINE_PRECIS, // 输出精度
            CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, // 抗锯齿
            VARIABLE_PITCH,
            TEXT("微软雅黑")); // 字体名称
        SelectObject(hdc, hFont);

        // 步骤3：设置文字属性
        SetTextColor(hdc, RGB(255, 0, 0)); // 红色文字
        SetBkMode(hdc, TRANSPARENT);       // 透明背景模式

        // 步骤4：绘制居中文本
        RECT rect;
        GetClientRect(hWnd, &rect);
        DrawText(hdc, studentName.c_str(), -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        // 清理资源
        DeleteObject(hFont);
        EndPaint(hWnd, &ps);
        break;
    }

    // 系统托盘消息处理
    case WM_USER + 1:
        switch (LOWORD(lParam))
        {
        case WM_RBUTTONUP: // 右键单击：显示上下文菜单
        {
            // 创建弹出菜单
            HMENU hMenu = CreatePopupMenu();
            AppendMenu(hMenu, MF_STRING, 1001, L"修改名字"); // 修改命令
            AppendMenu(hMenu, MF_STRING, IDM_EXIT, L"退出"); // 退出命令

            // 显示菜单
            POINT pt;
            GetCursorPos(&pt);
            SetForegroundWindow(hWnd); // 确保菜单获得焦点
            TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
            DestroyMenu(hMenu);
            break;
        }
        case WM_LBUTTONDBLCLK: // 双击左键：恢复窗口显示
            ShowWindow(hWnd, SW_SHOW);
            SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            break;
        }
        break;

    // 窗口层级变化处理（保持置顶）
    case WM_WINDOWPOSCHANGED: {
        WINDOWPOS* pos = (WINDOWPOS*)lParam;
        if (!(pos->flags & SWP_NOZORDER)) // 如果层级发生变化
        {
            SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        }
        break;
    }

    // 定时器消息（每0.5秒重置置顶状态）
    case WM_TIMER:
        if (!g_bDialogActive) // 对话框未激活时才操作
        {
            SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        }
        break;

    // 命令处理（菜单选项）
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

    // 窗口销毁处理
    case WM_DESTROY:
        KillTimer(hWnd, 1);                 // 销毁定时器
        Shell_NotifyIcon(NIM_DELETE, &nid); // 移除托盘图标
        PostQuitMessage(0);                 // 退出消息循环
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 修改姓名对话框处理
//--------------------------------------------------
void UpdateName(HWND hWnd)
{
    WCHAR name[MAX_LOADSTRING] = {0};
    wcscpy_s(name, studentName.c_str()); // 初始化编辑框内容

    // 分配内存传递姓名数据
    WCHAR* pName = (WCHAR*)GlobalAlloc(GPTR, MAX_LOADSTRING * sizeof(WCHAR));
    if (!pName)
    {
        MessageBox(hWnd, L"内存分配失败", L"错误", MB_ICONERROR);
        return;
    }
    wcscpy_s(pName, MAX_LOADSTRING, name);

    // 保存当前置顶状态
    BOOL wasTopmost = (GetWindowLongPtr(hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST);

    // 临时取消置顶（避免对话框被遮盖）
    SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    g_bDialogActive = TRUE; // 标记对话框激活状态

    // 创建模态对话框
    INT_PTR result = DialogBoxParam(
        hInst,
        MAKEINTRESOURCE(IDD_DIALOG1),
        hWnd,
        [](HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) -> INT_PTR {
            switch (message)
            {
            case WM_INITDIALOG:
                // 初始化编辑框内容
                SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
                SetDlgItemText(hDlg, IDC_EDIT1, (WCHAR*)lParam);
                return TRUE;

            case WM_COMMAND:
                switch (LOWORD(wParam))
                {
                case IDOK: // 确认按钮
                {
                    WCHAR* pName = (WCHAR*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
                    GetDlgItemText(hDlg, IDC_EDIT1, pName, MAX_LOADSTRING);
                    studentName = pName; // 更新姓名

                    // 触发窗口重绘
                    InvalidateRect(GetParent(hDlg), NULL, TRUE);
                    UpdateWindow(GetParent(hDlg));
                    EndDialog(hDlg, IDOK);
                    return TRUE;
                }
                case IDCANCEL: // 取消按钮
                    EndDialog(hDlg, IDCANCEL);
                    return TRUE;
                }
                break;
            }
            return FALSE;
        },
        (LPARAM)pName);

    // 恢复状态
    g_bDialogActive = FALSE;
    if (wasTopmost)
    {
        SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }

    GlobalFree(pName); // 释放内存
}
