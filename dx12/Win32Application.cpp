#include "Win32Application.h"

// DXApplication�|�C���^���E�B���h�E�̃��[�U�[�f�[�^�ɕێ����邽�߂̊֐�
void SetDxAppPtr(HWND hwnd, DXApplication* dxApp) {
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(dxApp));
}

DXApplication* GetDxAppPtr(HWND hwnd) {
    return reinterpret_cast<DXApplication*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
}

void Win32Application::Run(DXApplication* dxApp, HINSTANCE hInstance) {
    // --- �E�B���h�E�N���X���� ---
    WNDCLASSEX windowClass = {};
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = _T("DXSampleClass");
    RegisterClassEx(&windowClass);

    // --- �E�B���h�E�T�C�Y���� ---
    RECT windowRect = { 0, 0, dxApp->GetWindowWidth(), dxApp->GetWindowHeight() };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, false);

    // --- �E�B���h�E���� ---
    HWND hwnd = CreateWindow(
        windowClass.lpszClassName,
        dxApp->GetTitle(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr, nullptr, hInstance, nullptr);

    // DXApplication�|�C���^�����[�U�[�f�[�^�ɃZ�b�g
    SetDxAppPtr(hwnd, dxApp);

    // �A�v��������
    dxApp->OnInit(hwnd);

    // �e�N�X�`����������BGM�ǂݍ��݂͂����ł��
    dxApp->InitializeTexture(L"Assets/Image/mura.png", 0.0f, 0.0f, dxApp->GetWindowWidth(), dxApp->GetWindowHeight());
    dxApp->InitializeTexture(L"Assets/Image/kenshi.png", 0.0f, 0.0f, dxApp->GetWindowWidth(), dxApp->GetWindowHeight());

    if (!dxApp->engine_.Initialize()) {
        wprintf(L"���������s\n");
    }

    dxApp->engine_.LoadBGM(L"title", L"Assets/Audio/maou_bgm_karaoke_01_flower.mp3");
    dxApp->engine_.SetVolumeForBGM(L"title", 0.3f);
    dxApp->engine_.PlayBGM(L"title");

    // ���ʉ��ǂݍ��݁i�N���b�N�p�j
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

// WindowProc�̒��ŃN���b�N���m��SE�Đ����s��
LRESULT CALLBACK Win32Application::WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
    case WM_LBUTTONDOWN: {
        DXApplication* dxApp = GetDxAppPtr(hwnd);
        if (dxApp) {
            dxApp->engine_.PlaySE(L"click");
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
