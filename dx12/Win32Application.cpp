#include "Win32Application.h"

// DXApplicationポインタをウィンドウのユーザーデータに保持するための関数
void SetDxAppPtr(HWND hwnd, DXApplication* dxApp) {
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(dxApp));
}

DXApplication* GetDxAppPtr(HWND hwnd) {
    return reinterpret_cast<DXApplication*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
}

void Win32Application::Run(DXApplication* dxApp, HINSTANCE hInstance) {
    // --- ウィンドウクラス生成 ---

    WNDCLASSEX windowClass = {};
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = hInstance;
    windowClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    windowClass.lpszMenuName = nullptr;
    windowClass.lpszClassName = L"DXSampleClass";
    windowClass.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

    // ★★ クラス登録を忘れずに！ ★★
    if (!RegisterClassEx(&windowClass)) {
        MessageBoxW(nullptr, L"RegisterClassEx failed", L"Error", MB_OK);
        return ;
    }

    // --- ウィンドウサイズ調整 ---
    RECT windowRect = { 0, 0, dxApp->GetWindowWidth(), dxApp->GetWindowHeight() };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, false);

    HWND hwnd = CreateWindow(
        windowClass.lpszClassName,
        dxApp->GetTitle(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr, nullptr, hInstance, nullptr
    );

    if (!hwnd) {
        MessageBoxW(nullptr, L"CreateWindow failed", L"Error", MB_OK);
        return ;
    }

    // DXApplicationポインタをユーザーデータにセット
    SetDxAppPtr(hwnd, dxApp);

    // アプリ初期化
    dxApp->OnInit(hwnd);

    // テクスチャ初期化やBGM読み込みはここでやる
    dxApp->InitializeTexture(L"mura", L"Assets/Image/mura.png", 0.0f, 0.0f, dxApp->GetWindowWidth(), dxApp->GetWindowHeight());
    dxApp->InitializeTexture(L"kenshi",L"Assets/Image/kenshi.png", 0.0f, 0.0f, dxApp->GetWindowWidth(), dxApp->GetWindowHeight());

    if (!dxApp->engine_.Initialize()) {
        wprintf(L"初期化失敗\n");
    }

    dxApp->engine_.LoadBGM(L"title", L"Assets/Audio/maou_bgm_karaoke_01_flower.mp3");
    dxApp->engine_.SetVolumeForBGM(L"title", 0.3f);
    dxApp->engine_.PlayBGM(L"title");

    // 効果音読み込み（クリック用）
    dxApp->engine_.LoadSE(L"click", L"Assets/Audio/maou_se_system49.mp3");
    dxApp->engine_.SetVolumeForSE(L"click", 1.0f);

    ShowWindow(hwnd, SW_SHOW);

    MSG msg = {};
    float angle = 0.0f;

    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        dxApp->OnUpdate();
        dxApp->OnRender();
    }

    dxApp->OnDestroy();

    UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
}

// WindowProcの中でクリック検知とSE再生を行う
LRESULT CALLBACK Win32Application::WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
    case WM_LBUTTONDOWN: {
        DXApplication* dxApp = GetDxAppPtr(hwnd);
        if (dxApp) {
            dxApp->engine_.PlaySE(L"click");
            dxApp->SetTextureVisible(L"kenshi", !dxApp->GetTextureVisible(L"kenshi"));
        }
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, message, wparam, lparam);
    }
}
