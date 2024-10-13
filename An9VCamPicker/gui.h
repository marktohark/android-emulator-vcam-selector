#include <Windows.h>
#pragma comment(lib, "gdi32.lib")

#ifndef GUI_H
#define GUI_H

LRESULT CALLBACK TrashParentWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_WINDOWPOSCHANGING:
        return 0;
    case WM_CLOSE:
        HANDLE myself;
        myself = OpenProcess(PROCESS_ALL_ACCESS, false, GetCurrentProcessId());
        TerminateProcess(myself, 0);

        return true;

        break;

    default:
        break;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

HWND hwnd = NULL;
typedef HWND(WINAPI* CreateWindowInBand)(_In_ DWORD dwExStyle, _In_opt_ ATOM atom, _In_opt_ LPCWSTR lpWindowName, _In_ DWORD dwStyle, _In_ int X, _In_ int Y, _In_ int nWidth, _In_ int nHeight, _In_opt_ HWND hWndParent, _In_opt_ HMENU hMenu, _In_opt_ HINSTANCE hInstance, _In_opt_ LPVOID lpParam, DWORD band);

void CreateWin(HMODULE hModule, UINT zbid, const wchar_t* title, const wchar_t* classname)
{
    {
        HINSTANCE hInstance = hModule;

        WNDCLASSEX wndParentClass = {};
        wndParentClass.cbSize = sizeof(WNDCLASSEX);

        wndParentClass.cbClsExtra = 0;
        wndParentClass.hIcon = NULL;
        wndParentClass.lpszMenuName = NULL;
        wndParentClass.hIconSm = NULL;
        wndParentClass.lpfnWndProc = TrashParentWndProc;
        wndParentClass.hInstance = hInstance;
        wndParentClass.style = CS_HREDRAW | CS_VREDRAW;
        wndParentClass.hCursor = LoadCursor(0, IDC_ARROW);
        wndParentClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
        wndParentClass.lpszClassName = classname;

        auto res = RegisterClassEx(&wndParentClass);

        const auto hpath = LoadLibrary(L"user32.dll");
        const auto pCreateWindowInBand = CreateWindowInBand(GetProcAddress(hpath, "CreateWindowInBand"));

        auto hwndParent = pCreateWindowInBand(WS_EX_TOPMOST | WS_EX_NOACTIVATE,
            res,
            NULL,
            0x80000000,
            0, 0, 0, 0,
            NULL,
            NULL,
            wndParentClass.hInstance,
            LPVOID(res),
            zbid);

        SetWindowLong(hwndParent, GWL_STYLE, 0);
        SetWindowLong(hwndParent, GWL_EXSTYLE, 0);

        SetWindowPos(hwndParent, nullptr, 40, 40, 600, 600, SWP_SHOWWINDOW | SWP_NOZORDER);
        ShowWindow(hwndParent, SW_SHOW);
        UpdateWindow(hwndParent);

        if (hwndParent != nullptr)
            hwnd = hwndParent;
    }
}

DWORD WINAPI InstallGUI(LPVOID lpParam) {
    CreateWin(NULL, 16, L"Really Genuine Window++", L"TestPlus");


    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
#endif