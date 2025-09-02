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
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = hInstance;
    windowClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    windowClass.lpszMenuName = nullptr;
    windowClass.lpszClassName = L"DXSampleClass";
    windowClass.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

    // ���� �N���X�o�^��Y�ꂸ�ɁI ����
    if (!RegisterClassEx(&windowClass)) {
        MessageBoxW(nullptr, L"RegisterClassEx failed", L"Error", MB_OK);
        return ;
    }

    // --- �E�B���h�E�T�C�Y���� ---
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

    // DXApplication�|�C���^�����[�U�[�f�[�^�ɃZ�b�g
    SetDxAppPtr(hwnd, dxApp);

    // �A�v��������
    dxApp->OnInit(hwnd);

    enum GameState
    {
        TITLE,
        PLAY,
        PAUSE,
        END,
    };

    GameState gs = GameState::TITLE;

    // �e�N�X�`����������BGM�ǂݍ��݂͂����ł��
    dxApp->InitializeTexture(L"mura", L"Assets/Image/mura.png", 0.0f, 0.0f, dxApp->GetWindowWidth(), dxApp->GetWindowHeight(), 1.0f);
    dxApp->InitializeTexture(L"kenshi",L"Assets/Image/kenshi.png", 0.0f, 0.0f, dxApp->GetWindowWidth(), dxApp->GetWindowHeight(), 1.0f);

    if (!dxApp->engine_.Initialize()) {
        wprintf(L"���������s\n");
    }

    dxApp->engine_.LoadBGM(L"title", L"Assets/Audio/maou_bgm_karaoke_01_flower.mp3");
    dxApp->engine_.SetVolumeForBGM(L"title", 0.3f);
    dxApp->engine_.PlayBGM(L"title");

    // ���ʉ��ǂݍ��݁i�N���b�N�p�j
    dxApp->engine_.LoadSE(L"click", L"Assets/Audio/maou_se_system49.mp3");
    dxApp->engine_.SetVolumeForSE(L"click", 1.0f);

    std::wstring titleButton = L"titleButton";
    dxApp->buttons[titleButton] = Button(titleButton, L"Assets/Image/titleButton.png", dxApp->GetWindowWidth()/2-150.0f, dxApp->GetWindowHeight()/2+100.0f, 300.0f, 100.0f, dxApp->GetWindowWidth(), dxApp->GetWindowHeight(), [&gs]() { gs = GameState::PLAY; });
    dxApp->InitializeTexture(
        dxApp->buttons[titleButton].getKey(),
        dxApp->buttons[titleButton].getImagePath(),
        dxApp->buttons[titleButton].getXAbs(dxApp->GetWindowWidth()),   // x_ratio �� ��Βl
        dxApp->buttons[titleButton].getYAbs(dxApp->GetWindowHeight()),   // y_ratio �� ��Βl
        dxApp->buttons[titleButton].getWidthAbs(dxApp->GetWindowWidth()),
        dxApp->buttons[titleButton].getHeightAbs(dxApp->GetWindowHeight()),
        0.5f  // alpha
    );
    ShowWindow(hwnd, SW_SHOW);

    MSG msg = {};
    float angle = 0.0f;

    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        dxApp->OnUpdate();

        // ����ŃJ�[�\������
        std::for_each(std::execution::par, dxApp->buttons.begin(), dxApp->buttons.end(),
            [hwnd](auto& btPair) {
                btPair.second.AreaChack(hwnd); // ���肾��
            }
        );

        // �����Ń��C���X���b�h�Ŗ��邳�E�A���t�@�X�V
        for (auto& btPair : dxApp->buttons) {
            if (btPair.second.AreaChack(hwnd)) { // ��: ��قǂ̔��茋�ʂ��g�p
                dxApp->SetTextureBrightnessAndAlpha(btPair.second.getKey(), 1.5f, 0.8f);
            }
            else {
                dxApp->SetTextureBrightnessAndAlpha(btPair.second.getKey(), 1.0f, 0.5f);
            }
        }

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
            for (auto &bt : dxApp->buttons) {
                bt.second.Chack(hwnd);
            }
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
