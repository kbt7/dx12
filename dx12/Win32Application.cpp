#include "Win32Application.h"
#include <execution>
#include <chrono>
#include <string>
#include <vector>

// --- DXApplication�|�C���^�Ǘ� ---
void SetDxAppPtr(HWND hwnd, DXApplication* dxApp) {
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(dxApp));
}
DXApplication* GetDxAppPtr(HWND hwnd) {
    return reinterpret_cast<DXApplication*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
}

// --- �V�[���������֐� ---
void InitScene_Title(DXApplication* dxApp, PetalSystem& petalSystem) {
    // �^�C�g���w�i
    dxApp->InitializeTexture(L"mura", L"Assets/Image/mura.png",
        0.0f, 0.0f, dxApp->GetWindowWidth(), dxApp->GetWindowHeight(), 1.0f);

    // �^�C�g���{�^��
    std::wstring titleButton = L"titleButton";
    dxApp->buttons[titleButton] = Button(
        titleButton, L"Assets/Image/titleButton.png",
        dxApp->GetWindowWidth() / 2 - 150.0f, dxApp->GetWindowHeight() / 2 + 100.0f,
        300.0f, 100.0f, dxApp->GetWindowWidth(), dxApp->GetWindowHeight(),
        []() {} // �R�[���o�b�N�͌�Őݒ�
    );

    dxApp->InitializeTexture(
        dxApp->buttons[titleButton].getKey(),
        dxApp->buttons[titleButton].getImagePath(),
        dxApp->buttons[titleButton].getXAbs(dxApp->GetWindowWidth()),
        dxApp->buttons[titleButton].getYAbs(dxApp->GetWindowHeight()),
        dxApp->buttons[titleButton].getWidthAbs(dxApp->GetWindowWidth()),
        dxApp->buttons[titleButton].getHeightAbs(dxApp->GetWindowHeight()),
        0.5f);

    // BGM�E���ʉ�
    dxApp->engine_.LoadBGM(L"title", L"Assets/Audio/maou_bgm_karaoke_01_flower.mp3");
    dxApp->engine_.SetVolumeForBGM(L"title", 0.3f);
    dxApp->engine_.PlayBGM(L"title");
    dxApp->engine_.LoadSE(L"click", L"Assets/Audio/maou_se_system49.mp3");
    dxApp->engine_.SetVolumeForSE(L"click", 1.0f);

    // �Ԃт珉����
    std::vector<std::wstring> petalImages = { L"Assets/Image/petal.png" };
    petalSystem.Initialize(petalImages, 100);
}

void InitScene_Play(DXApplication* dxApp, PetalSystem& petalSystem) {
    // �^�C�g���p���\�[�X�j��
    dxApp->ReleaseTexture(L"mura");
    dxApp->ReleaseTexture(L"titleButton");
    dxApp->buttons.clear();
    petalSystem.Clear();

    // �v���C�p�摜
    dxApp->InitializeTexture(L"kenshi", L"Assets/Image/kenshi.png",
        0.0f, 0.0f, dxApp->GetWindowWidth(), dxApp->GetWindowHeight(), 1.0f);

    // �K�v�Ȃ�BGM�ؑ֓���������
}

// --- ���C�����s ---
void Win32Application::Run(DXApplication* dxApp, HINSTANCE hInstance) {
    // �E�B���h�E�N���X����
    WNDCLASSEX windowClass = {};
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    windowClass.lpszClassName = L"DXSampleClass";
    windowClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    windowClass.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

    if (!RegisterClassEx(&windowClass)) {
        MessageBoxW(nullptr, L"RegisterClassEx failed", L"Error", MB_OK);
        return;
    }

    RECT windowRect = { 0,0,dxApp->GetWindowWidth(), dxApp->GetWindowHeight() };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, false);

    HWND hwnd = CreateWindow(
        windowClass.lpszClassName, dxApp->GetTitle(),
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT,
        windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
        nullptr, nullptr, hInstance, nullptr);
    if (!hwnd) {
        MessageBoxW(nullptr, L"CreateWindow failed", L"Error", MB_OK);
        return;
    }

    SetDxAppPtr(hwnd, dxApp);
    dxApp->OnInit(hwnd);

    enum GameState { TITLE, PLAY, PAUSE, END };
    GameState gs = TITLE;

    if (!dxApp->engine_.Initialize()) {
        wprintf(L"���������s\n");
    }

    PetalSystem petalSystem(dxApp, dxApp->GetWindowWidth(), dxApp->GetWindowHeight());
    InitScene_Title(dxApp, petalSystem);

    // �{�^���̃R�[���o�b�N���ォ��ݒ�
    dxApp->buttons[L"titleButton"].SetCallback([&]() {
        petalSystem.StartFadeOut(2.0f); // �t�F�[�h�J�n
        });

    ShowWindow(hwnd, SW_SHOW);

    MSG msg = {};
    auto previousTime = std::chrono::high_resolution_clock::now();

    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - previousTime).count();
        previousTime = currentTime;

        dxApp->OnUpdate();

        if (gs == TITLE) {
            petalSystem.Update(deltaTime);
            petalSystem.Render();

            // �t�F�[�h�A�E�g������v���C��ʏ�����
            if (petalSystem.IsFinished()) {
                InitScene_Play(dxApp, petalSystem);
                gs = PLAY;
            }
        }
        else if (gs == PLAY) {
            dxApp->OnRender();
        }

        // ����ŃJ�[�\������
        std::for_each(std::execution::par, dxApp->buttons.begin(), dxApp->buttons.end(),
            [hwnd](auto& btPair) { btPair.second.AreaCheck(hwnd); });

        for (auto& btPair : dxApp->buttons) {
            if (btPair.second.AreaCheck(hwnd)) {
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

// --- �N���b�N���m ---
LRESULT CALLBACK Win32Application::WindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_LBUTTONDOWN: {
        DXApplication* dxApp = GetDxAppPtr(hwnd);
        if (dxApp) {
            dxApp->engine_.PlaySE(L"click");
            for (auto& bt : dxApp->buttons) bt.second.Check(hwnd);
        }
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, msg, wp, lp);
    }
}

HWND Win32Application::AddSelectableText(HWND hwndParent, HINSTANCE hInstance,
    int x, int y, int w, int h, const std::wstring& text)
{
    HWND hEdit = CreateWindowExW(0, L"EDIT", text.c_str(),
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY | ES_MULTILINE | ES_AUTOVSCROLL,
        x, y, w, h, hwndParent, nullptr, hInstance, nullptr);

    SendMessageW(hEdit, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
    return hEdit;
}
