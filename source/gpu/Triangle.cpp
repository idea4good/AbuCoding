// cl Triangle.cpp user32.lib

#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
using namespace DirectX;

struct ConstantBuffer {
    XMMATRIX transform;
};

ID3D11Buffer* g_constantBuffer = nullptr;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "user32.lib")

HWND g_hWnd = nullptr;
ID3D11Device* g_device = nullptr;
ID3D11DeviceContext* g_context = nullptr;
IDXGISwapChain* g_swapChain = nullptr;
ID3D11RenderTargetView* g_rtv = nullptr;
ID3D11Buffer* g_vertexBuffer = nullptr;
ID3D11VertexShader* g_vs = nullptr;
ID3D11PixelShader* g_ps = nullptr;
ID3D11InputLayout* g_layout = nullptr;

struct Vertex {
    float x, y, z;
    float r, g, b, a;
};

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_DESTROY) { PostQuitMessage(0); return 0; }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

bool InitWindow(HINSTANCE hInstance) {
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = (LPCSTR)L"DXWindow";
    RegisterClass(&wc);

    g_hWnd = CreateWindowEx(0, wc.lpszClassName, (LPCSTR)L"Triangle",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        nullptr, nullptr, hInstance, nullptr);
    ShowWindow(g_hWnd, SW_SHOW);
    return true;
}

bool InitD3D() {
    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferDesc.Width = 800;
    scd.BufferDesc.Height = 600;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.SampleDesc.Count = 1;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.BufferCount = 1;
    scd.OutputWindow = g_hWnd;
    scd.Windowed = FALSE; // 全屏
    scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE,
        nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &scd,
        &g_swapChain, &g_device, nullptr, &g_context))) return false;

    ID3D11Texture2D* backBuffer;
    g_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    g_device->CreateRenderTargetView(backBuffer, nullptr, &g_rtv);
    backBuffer->Release();
    g_context->OMSetRenderTargets(1, &g_rtv, nullptr);

    D3D11_VIEWPORT vp = { 0, 0, 800.0f, 600.0f, 0.0f, 1.0f };
    g_context->RSSetViewports(1, &vp);
    return true;
}

bool InitTriangle() {
    Vertex vertices[] = {
        { 0.0f,  0.5f, 0.0f, 1, 0, 0, 1 },
        { 0.5f, -0.5f, 0.0f, 0, 1, 0, 1 },
        {-0.5f, -0.5f, 0.0f, 0, 0, 1, 1 },
    };

    D3D11_BUFFER_DESC bd = { sizeof(vertices), D3D11_USAGE_DEFAULT,
        D3D11_BIND_VERTEX_BUFFER, 0, 0, 0 };
    D3D11_SUBRESOURCE_DATA initData = { vertices };
    g_device->CreateBuffer(&bd, &initData, &g_vertexBuffer);

    D3D11_BUFFER_DESC cbd = {};
    cbd.ByteWidth = sizeof(ConstantBuffer);
    cbd.Usage = D3D11_USAGE_DEFAULT;
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    g_device->CreateBuffer(&cbd, nullptr, &g_constantBuffer);

    const char* vsCode =
    "cbuffer ConstantBuffer : register(b0) { matrix transform; };"
    "struct VS_IN { float3 pos : POSITION; float4 col : COLOR; };"
    "struct PS_IN { float4 pos : SV_POSITION; float4 col : COLOR; };"
    "PS_IN VS(VS_IN input) {"
    "  PS_IN output;"
    "  output.pos = mul(float4(input.pos,1), transform);"
    "  output.col = input.col;"
    "  return output;"
    "}";

    const char* psCode =
        "struct PS_IN { float4 pos : SV_POSITION; float4 col : COLOR; };"
        "float4 PS(PS_IN input) : SV_Target { return input.col; }";

    ID3DBlob* vsBlob = nullptr, * psBlob = nullptr;
    D3DCompile(vsCode, strlen(vsCode), nullptr, nullptr, nullptr,
        "VS", "vs_4_0", 0, 0, &vsBlob, nullptr);
    D3DCompile(psCode, strlen(psCode), nullptr, nullptr, nullptr,
        "PS", "ps_4_0", 0, 0, &psBlob, nullptr);

    g_device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &g_vs);
    g_device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &g_ps);

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
          D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,
          D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    g_device->CreateInputLayout(layout, 2, vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(), &g_layout);

    vsBlob->Release(); psBlob->Release();
    return true;
}

void Render(bool isRenderEnabled) {
    static float angle = 0.0f;
    angle += 0.01f; // 每帧旋转一点
    XMMATRIX rotation = XMMatrixRotationZ(angle);
    ConstantBuffer cb;
    cb.transform = XMMatrixTranspose(rotation); // 转置以匹配 HLSL 行主序
    g_context->UpdateSubresource(g_constantBuffer, 0, nullptr, &cb, 0, 0);
    g_context->VSSetConstantBuffers(0, 1, &g_constantBuffer);

    float clearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    g_context->ClearRenderTargetView(g_rtv, clearColor);

    UINT stride = sizeof(Vertex), offset = 0;
    g_context->IASetVertexBuffers(0, 1, &g_vertexBuffer, &stride, &offset);
    g_context->IASetInputLayout(g_layout);
    g_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    g_context->VSSetShader(g_vs, nullptr, 0);
    g_context->PSSetShader(g_ps, nullptr, 0);
    g_context->Draw(3, 0);

    if(isRenderEnabled) { g_swapChain->Present(1, 0); }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    if (!InitWindow(hInstance)) return -1;
    if (!InitD3D()) return -1;
    if (!InitTriangle()) return -1;

    bool isRenderEnabled = true;
    MSG msg = {};
    while (msg.message != WM_QUIT) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            PostQuitMessage(0);
        }

        if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
            isRenderEnabled = !isRenderEnabled;
        }

        Render(isRenderEnabled);        
        Sleep(16);
    }
    return 0;
}
