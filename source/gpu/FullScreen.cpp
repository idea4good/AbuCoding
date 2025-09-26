// cl FullScreen.cpp user32.lib

#include <windows.h>
#include <d3d11.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "user32.lib")

HWND g_hWnd = nullptr;
ID3D11Device* g_device = nullptr;
ID3D11DeviceContext* g_context = nullptr;
IDXGISwapChain* g_swapChain = nullptr;
ID3D11RenderTargetView* g_rtv = nullptr;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_DESTROY) { PostQuitMessage(0); return 0; }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

bool InitWindow(HINSTANCE hInstance) {
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = (LPCSTR)L"DX11Window";
    RegisterClass(&wc);

    g_hWnd = CreateWindowEx(0, wc.lpszClassName, (LPCSTR)L"DirectX Fullscreen",
        WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
        nullptr, nullptr, hInstance, nullptr);

    if (!g_hWnd) return false;
    ShowWindow(g_hWnd, SW_SHOW);
    return true;
}

bool InitD3D() {
    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferDesc.Width = GetSystemMetrics(SM_CXSCREEN);
    scd.BufferDesc.Height = GetSystemMetrics(SM_CYSCREEN);
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferDesc.RefreshRate.Numerator = 60;
    scd.BufferDesc.RefreshRate.Denominator = 1;
    scd.SampleDesc.Count = 1;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.BufferCount = 1;
    scd.OutputWindow = g_hWnd;
    scd.Windowed = FALSE; // 全屏
    scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    if (FAILED(D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0,
        D3D11_SDK_VERSION, &scd, &g_swapChain, &g_device, nullptr, &g_context)))
        return false;

    ID3D11Texture2D* backBuffer = nullptr;
    g_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    g_device->CreateRenderTargetView(backBuffer, nullptr, &g_rtv);
    backBuffer->Release();

    g_context->OMSetRenderTargets(1, &g_rtv, nullptr);

    D3D11_VIEWPORT vp = {};
    vp.Width = (FLOAT)GetSystemMetrics(SM_CXSCREEN);
    vp.Height = (FLOAT)GetSystemMetrics(SM_CYSCREEN);
    vp.MinDepth = 0.0f; vp.MaxDepth = 1.0f;
    g_context->RSSetViewports(1, &vp);

    return true;
}

void Render() {
    static int index;
    float blueColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f }; // 蓝色背景
    float redColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f }; // 红色背景
    g_context->ClearRenderTargetView(g_rtv, (index++%2)?blueColor:redColor);
    g_swapChain->Present(1, 0); // VSync
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    if (!InitWindow(hInstance)) return -1;
    if (!InitD3D()) return -1;

    MSG msg = {};
    while (msg.message != WM_QUIT) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // 检测 ESC 键是否按下
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            PostQuitMessage(0); // 触发退出
        }

        Render();
        Sleep(20);
    }

    return 0;
}
