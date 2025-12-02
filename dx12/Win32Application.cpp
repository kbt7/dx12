#include "Win32Application.h"
#include <execution>
#include <chrono>
#include <string>
#include <vector>
#include <sstream>  // std::wstringstream, std::wostringstream
#include <locale>  // std::locale
#include <windows.h> // Win32 APIの基本
#include <commctrl.h> // Editコントロール用

// Win32Application静的メンバの定義（クラス外での実体定義）
Win32Application::GameState Win32Application::gs_ = Win32Application::TITLE;
DXApplication* Win32Application::dxApp_ = nullptr;
HWND Win32Application::hDetailText_ = nullptr;
HINSTANCE Win32Application::s_hInstance = nullptr;
HWND Win32Application::s_hwnd = nullptr;

// ★ 新規追加: GoToPlay と GoToTavern の静的メンバの定義
std::function<void()> Win32Application::GoToPlay_ = nullptr;
std::function<void(std::function<void()>)> Win32Application::GoToTavern_ = nullptr;

// --- DXApplicationポインタ管理 ---
void SetDxAppPtr(HWND hwnd, DXApplication* dxApp) {
	SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(dxApp));
}
DXApplication* GetDxAppPtr(HWND hwnd) {
	// 【重要】GetWindowLongPtrを使う前に、hwndの基本的な有効性チェックを行う。
	// 無効なポインタ（0xdddd...）を渡すと、OS関数がクラッシュを引き起こす可能性がある。
	if (hwnd == nullptr || !IsWindow(hwnd)) {
		return nullptr;
	}
	return reinterpret_cast<DXApplication*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
}

// --- シーン初期化関数 ---
void InitScene_Title(DXApplication* dxApp, PetalSystem& petalSystem) {
	// 重要な初期化：dxApp_静的メンバを設定
	Win32Application::dxApp_ = dxApp;

	// タイトル背景
	dxApp->InitializeTexture(L"mura", L"Assets/Image/mura.png",
		0.0f, 0.0f, dxApp->GetWindowWidth(), dxApp->GetWindowHeight(), 1.0f);

	// タイトルボタン
	std::wstring titleButton = L"titleButton";
	dxApp->buttons[titleButton] = Button(
		titleButton, L"Assets/Image/titleButton.png",
		dxApp->GetWindowWidth() / 2 - 150.0f, dxApp->GetWindowHeight() / 2 + 100.0f,
		300.0f, 100.0f, dxApp->GetWindowWidth(), dxApp->GetWindowHeight(),
		[]() {} // コールバックは後で設定
	);

	dxApp->InitializeTexture(
		dxApp->buttons[titleButton].getKey(),
		dxApp->buttons[titleButton].getImagePath(),
		dxApp->buttons[titleButton].getXAbs(dxApp->GetWindowWidth()),
		dxApp->buttons[titleButton].getYAbs(dxApp->GetWindowHeight()),
		dxApp->buttons[titleButton].getWidthAbs(dxApp->GetWindowWidth()),
		dxApp->buttons[titleButton].getHeightAbs(dxApp->GetWindowHeight()),
		0.5f);

	// BGM・効果音
	dxApp->engine_.LoadBGM(L"title", L"Assets/Audio/maou_bgm_karaoke_01_flower.mp3");
	dxApp->engine_.SetVolumeForBGM(L"title", 0.3f);
	dxApp->engine_.PlayBGM(L"title");
	dxApp->engine_.LoadSE(L"click", L"Assets/Audio/maou_se_system49.mp3");
	dxApp->engine_.SetVolumeForSE(L"click", 1.0f);

	// 花びら初期化
	std::vector<std::wstring> petalImages = { L"Assets/Image/petal.png" };
	petalSystem.Initialize(petalImages, 100);
}

void InitScene_Play(DXApplication* dxApp, PetalSystem& petalSystem) {
	// 重要な初期化：dxApp_静的メンバを設定
	Win32Application::dxApp_ = dxApp;

	// ★ボタンオブジェクトクリア: シーン切り替え時に既存のボタンをすべてクリアする
	dxApp->buttons.clear();

	// ここではリソース解放は行わない (GoToPlayまたはRun内で行う)

	// プレイ用画像
	dxApp->InitializeTexture(L"kenshi", L"Assets/Image/kenshi.png",
		0.0f, 0.0f, dxApp->GetWindowWidth(), dxApp->GetWindowHeight(), 1.0f);

	// 3つのボタンを配置
	float btnW = 200.0f;
	float btnH = 80.0f;
	float startX = (dxApp->GetWindowWidth() - (btnW * 3 + 40.0f)) / 2.0f;
	float y = dxApp->GetWindowHeight() * 0.7f;

	// 冒険ボタン
	std::wstring advBtn = L"btn_adventure";
	dxApp->buttons[advBtn] = Button(advBtn, L"Assets/Image/Button_Adventure.png",
		startX, y, btnW, btnH, dxApp->GetWindowWidth(), dxApp->GetWindowHeight(),
		[]() { /* 冒険処理 */ });
	dxApp->InitializeTexture(advBtn, L"Assets/Image/Button_Adventure.png",
		startX, y, btnW, btnH, 1.0f);

	// 酒場ボタン
	std::wstring pubBtn = L"btn_pub";
	dxApp->buttons[pubBtn] = Button(pubBtn, L"Assets/Image/Button_Pub.png",
		startX + btnW + 20.0f, y, btnW, btnH, dxApp->GetWindowWidth(), dxApp->GetWindowHeight(),
		[]() { /* 酒場処理 - コールバックはRun内で上書き */ });
	dxApp->InitializeTexture(pubBtn, L"Assets/Image/Button_Pub.png",
		startX + btnW + 20.0f, y, btnW, btnH, 1.0f);

	// セーブボタン
	std::wstring saveBtn = L"btn_save";
	dxApp->buttons[saveBtn] = Button(saveBtn, L"Assets/Image/Button_Save.png",
		startX + (btnW + 20.0f) * 2, y, btnW, btnH, dxApp->GetWindowWidth(), dxApp->GetWindowHeight(),
		[]() { /* セーブ処理 */ });
	dxApp->InitializeTexture(saveBtn, L"Assets/Image/Button_Save.png",
		startX + (btnW + 20.0f) * 2, y, btnW, btnH, 1.0f);
}


void InitScene_Tavern(DXApplication* dxApp, std::function<void(const std::wstring&)> onUnitSelect) {
	// 重要な初期化：dxApp_静的メンバを設定
	Win32Application::dxApp_ = dxApp;

	// ★ボタンオブジェクトクリア: シーン切り替え時に既存のボタンをすべてクリアする
	dxApp->buttons.clear();

	// 背景を少し暗くするなど（任意）
	// dxApp->SetTextureBrightnessAndAlpha(L"kenshi", 0.5f, 1.0f); 

	Game* game = Game::GetInstance();
	const auto& units = game->GetUnits();

	float x = 50.0f;
	float y = 50.0f;
	float size = 80.0f; // アイコンサイズ
	float gap = 20.0f;

	// ユニット一覧ボタンの生成
	for (const auto& pair : units) {
		const auto& unit = pair.second;
		std::wstring btnKey = L"unit_" + unit.id;

		// units.txtの画像パスにはフォルダが含まれていないため付与する
		std::wstring fullPath = L"Assets/Image/images/" + unit.imagePath;

		// ボタン作成
		dxApp->buttons[btnKey] = Button(
			btnKey, fullPath,
			x, y, size, size, dxApp->GetWindowWidth(), dxApp->GetWindowHeight(),
			[onUnitSelect, unit]() {
				// このユニットが選択された時の処理を実行
				onUnitSelect(unit.id);
			}
		);

		// テクスチャ読み込み
		dxApp->InitializeTexture(btnKey, fullPath, x, y, size, size, 1.0f);

		// 配置座標の更新 (折り返し処理)
		x += size + gap;
		if (x + size > dxApp->GetWindowWidth() * 0.6f) { // 画面の左側6割くらいにリスト表示
			x = 50.0f;
			y += size + gap;
		}
	}

	// 「戻る」ボタン
	std::wstring backBtn = L"btn_back";
	// 戻るボタン用の画像読み込み（既存のアセットを流用）
	dxApp->buttons[backBtn] = Button(backBtn, L"Assets/Image/Button_Adventure.png",
		dxApp->GetWindowWidth() - 250.0f, dxApp->GetWindowHeight() - 100.0f, 200.0f, 80.0f,
		dxApp->GetWindowWidth(), dxApp->GetWindowHeight(),
		[]() {}); // コールバックはRun内で設定

	dxApp->InitializeTexture(backBtn, L"Assets/Image/Button_Adventure.png",
		dxApp->GetWindowWidth() - 250.0f, dxApp->GetWindowHeight() - 100.0f, 200.0f, 80.0f, 1.0f);
}

void Win32Application::Run(DXApplication* dxApp, HINSTANCE hInstance) {

	s_hInstance = hInstance;
	dxApp_ = dxApp; // 静的メンバの設定

	// ゲームデータ読み込み
	Game* game = Game::GetInstance();
	game->LoadUnitData(L"Assets/Data/units.txt");
	game->LoadEnemyData(L"Assets/Data/EnemyData.txt");
	game->LoadItemData(L"Assets/Data/ItemData.txt");

	// --- ウィンドウ生成処理 ---
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
	RegisterClassEx(&windowClass);

	RECT windowRect = { 0, 0, (LONG)dxApp->GetWindowWidth(), (LONG)dxApp->GetWindowHeight() };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, false);

	HWND hwnd = CreateWindow(
		windowClass.lpszClassName, dxApp->GetTitle(),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CW_USEDEFAULT, CW_USEDEFAULT,
		windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
		nullptr, nullptr, hInstance, nullptr);

	// 【重要】作成されたHWNDを静的メンバーに保存
	if (hwnd == nullptr) {
		OutputDebugString(L"FATAL ERROR: CreateWindow failed.\n");
		return; // ウィンドウ作成失敗時は即座に終了
	}
	s_hwnd = hwnd; // ★ s_hwnd に保存

	SetDxAppPtr(hwnd, dxApp);
	dxApp->OnInit(hwnd);
	// ------------------------------------

	if (!dxApp->engine_.Initialize()) {
		OutputDebugString(L"Audio Init Failed\n");
	}

	PetalSystem petalSystem(dxApp, dxApp->GetWindowWidth(), dxApp->GetWindowHeight());
	InitScene_Title(dxApp, petalSystem);
	gs_ = Win32Application::TITLE;

	// タイトルからプレイへの初回遷移時のリソース解放フラグ
	bool titleResourcesReleased = false;

	// ★★★ GoToTavernの定義 (酒場への遷移) ★★★
	GoToTavern_ = [dxApp](std::function<void()> onBackToPlay) {

		if (!dxApp_) { OutputDebugString(L"ERROR: GoToTavern called with null dxApp_.\n"); return; }
		HWND currentHwnd = s_hwnd;
		if (!currentHwnd) { OutputDebugString(L"ERROR: GoToTavern called with null s_hwnd.\n"); return; }

		// Play画面のリソース（テクスチャ）を解放する (GoToPlay_でロードされたもの)
		dxApp_->ReleaseTexture(L"kenshi");
		dxApp_->ReleaseTexture(L"btn_adventure");
		dxApp_->ReleaseTexture(L"btn_pub");
		dxApp_->ReleaseTexture(L"btn_save");

		gs_ = Win32Application::TAVERN;

		// ★★★ ユニットクリック時のラムダ - 修正: メンバ名を speed に変更 ★★★
		InitScene_Tavern(dxApp, [currentHwnd](const std::wstring& id) {

			if (!dxApp_) { OutputDebugString(L"ERROR: Unit Selection called with null dxApp_.\n"); return; }
			if (!currentHwnd || !IsWindow(currentHwnd)) {
				OutputDebugString(L"ERROR: Unit Selection called with invalid currentHwnd.\n");
				return;
			}

			Unit* u = Game::GetInstance()->GetUnit(id);
			if (u) {
				// ... (詳細テキストの生成処理)
				std::wstringstream ss;
				ss.imbue(std::locale(""));
				ss << L"【名前】 " << u->name << L"\r\n"
					<< L"【種族】 " << u->race << L" / " << u->sex << L"\r\n"
					<< L"【HP】 " << u->hp << L"  【MP】 " << u->mp << L"\r\n"
					// ★ 修正: u->agility を u->speed に変更
					<< L"【攻撃】 " << u->attack << L"  【防御】 " << u->defence << L"  【素早さ】 " << u->speed << L"\r\n"
					<< L"【魔力】 " << u->magic << L"  【精神】 " << u->mental << L"\r\n\r\n"
					<< L"～詳細～\r\n" << u->detail;

				// 既存のテキストコントロールがあれば
				if (hDetailText_ && IsWindow(hDetailText_)) {
					// 破棄前に非表示にし、即座に再描画を要求することで、DXがその領域をクリアする機会を与える
					ShowWindow(hDetailText_, SW_HIDE);
					InvalidateRect(currentHwnd, NULL, TRUE);
					UpdateWindow(currentHwnd); // 同期的に再描画を実行

					DestroyWindow(hDetailText_);
					hDetailText_ = nullptr;
				}

				// スクロール可能なテキストコントロール (L"EDIT") を作成
				hDetailText_ = Win32Application::AddSelectableText(currentHwnd,
					(int)(dxApp_->GetWindowWidth() * 0.4f), 400,
					(int)(dxApp_->GetWindowWidth() * 0.55f), 300, ss.str());

				// テキストコントロールの切り替え後、親ウィンドウの再描画を強制
				InvalidateRect(currentHwnd, NULL, TRUE);

				// 顔グラフィックの表示
				const std::wstring textureKey = L"selected_face";
				std::wstring fullPath = L"Assets/Image/images/" + u->imagePath;
				// 既存のテクスチャがあれば解放してから再初期化
				dxApp_->ReleaseTexture(textureKey);
				dxApp_->InitializeTexture(textureKey, fullPath,
					dxApp_->GetWindowWidth() - 350.0f, 50.0f, 300.0f, 300.0f, 1.0f);
			}
			});

		// 戻るボタンの設定
		if (dxApp_->buttons.count(L"btn_back")) {
			dxApp_->buttons[L"btn_back"].SetCallback([onBackToPlay]() {

				if (!dxApp_) { OutputDebugString(L"ERROR: Back button clicked with null dxApp_.\n"); return; }

				// クリーンアップは GoToPlay_ の冒頭で行われるため、ここでは遷移のみ
				onBackToPlay(); // GoToPlayが実行され、その中で InitScene_Play が呼ばれ、ボタンがクリアされる
				});
		}
		};

	// ★★★ GoToPlayの定義 (プレイ画面への遷移) ★★★
	GoToPlay_ = [&petalSystem]() {

		if (!dxApp_) { OutputDebugString(L"ERROR: GoToPlay called with null dxApp_.\n"); return; }

		// 酒場画面で作成されたリソースとテクスチャをここで完全に解放する
		// テキスト、選択顔グラの解放
		if (hDetailText_ && IsWindow(hDetailText_)) {
			// 破棄前に非表示にして、即座に再描画する
			ShowWindow(hDetailText_, SW_HIDE);
			if (s_hwnd) { InvalidateRect(s_hwnd, NULL, TRUE); UpdateWindow(s_hwnd); }

			DestroyWindow(hDetailText_);
			hDetailText_ = nullptr;
		}
		dxApp_->ReleaseTexture(L"selected_face");

		// 【追加】酒場画面のリソース解放後、メインウィンドウの再描画を強制 
		if (s_hwnd) { InvalidateRect(s_hwnd, NULL, TRUE); }

		// 酒場画面で作成された動的なテクスチャ（ユニットアイコン、btn_back）をここで解放
		std::vector<std::wstring> keysToRelease;
		// ボタンオブジェクトはまだマップ上に残っているため、マップをイテレートしてキーを取得する
		for (const auto& pair : dxApp_->buttons) {
			keysToRelease.push_back(pair.first);
		}
		for (const auto& key : keysToRelease) {
			dxApp_->ReleaseTexture(key);
		}

		gs_ = Win32Application::PLAY;
		InitScene_Play(dxApp_, petalSystem);

		// PLAY画面のボタンコールバック設定
		if (dxApp_->buttons.count(L"btn_pub")) {
			// ... (GoToTavern_ の呼び出しは変更なし)
			dxApp_->buttons[L"btn_pub"].SetCallback([]() {
				if (!dxApp_) { OutputDebugString(L"ERROR: Pub button clicked with null dxApp_.\n"); return; }

				if (Win32Application::GoToTavern_) {
					Win32Application::GoToTavern_(Win32Application::GoToPlay_);
				}
				});
		}
		};

	// タイトルボタンのコールバック
	dxApp->buttons[L"titleButton"].SetCallback([&petalSystem]() {
		petalSystem.StartFadeOut(2.0f);
		dxApp_->buttons[L"titleButton"].SetCallback(nullptr);
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

		// ★★★ タイトルからプレイへの初回遷移時の処理 ★★★
		if (gs_ == Win32Application::TITLE) {
			petalSystem.Update(deltaTime);
			petalSystem.Render();

			if (petalSystem.IsFinished() && !titleResourcesReleased) {
				// タイトル画面のリソースをここで完全に解放 (テクスチャとボタンオブジェクト)
				dxApp->ReleaseTexture(L"mura");
				dxApp->ReleaseTexture(L"titleButton");
				dxApp->buttons.clear();
				petalSystem.Clear();
				titleResourcesReleased = true; // 解放済みフラグを立てる
				// 静的メンバの GoToPlay_ を使用する
				if (Win32Application::GoToPlay_) {
					Win32Application::GoToPlay_(); // プレイ画面へ遷移
				}
			}
		}
		else if (gs_ == Win32Application::PLAY) {
			// PLAY中の描画など
		}
		else if (gs_ == Win32Application::TAVERN) {
			// TAVERN中の描画
		}

		// ボタンのカーソル判定と演出（共通）
		// AreaCheckはWindowProc外で実行するため、並列実行(std::execution::par)は問題ない
		std::for_each(std::execution::par, dxApp->buttons.begin(), dxApp->buttons.end(),
			[hwnd](auto& btPair) {
				// 【重要】AreaCheckに渡す前にhwndの有効性チェック
				if (hwnd && IsWindow(hwnd)) {
					btPair.second.AreaCheck(hwnd);
				}
			});

		for (auto& btPair : dxApp->buttons) {
			if (btPair.second.AreaCheck(hwnd)) {
				dxApp->SetTextureBrightnessAndAlpha(btPair.second.getKey(), 1.2f, 1.0f); // ハイライト
			}
			else {
				if (btPair.first != L"selected_face") {
					dxApp->SetTextureBrightnessAndAlpha(btPair.second.getKey(), 1.0f, 1.0f);
				}
			}
		}

		dxApp->OnRender();
	}

	// 終了時のクリーンアップ
	// s_hwnd が有効であれば、DestroyWindow(s_hwnd) を呼び出すことも可能だが、
	// WM_QUITでメインループを抜けた場合、通常はOSが自動的にクリーンアップする。
	if (hDetailText_ && IsWindow(hDetailText_)) DestroyWindow(hDetailText_); // ★ 静的メンバを使用
	dxApp->OnDestroy();
	UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
}
// --- クリック検知 & テキストコントロールの透過処理 ---
LRESULT CALLBACK Win32Application::WindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	// 【重要】hwndが無効な場合の防御コード (クラッシュ回避)
	if (hwnd == nullptr || !IsWindow(hwnd)) {
		// 無効なHWNDが渡された場合、WM_DESTROY以外では処理をスキップ
		if (msg != WM_DESTROY && msg != WM_QUIT) {
			// 0xdddd... が渡された場合の安全策
			return 0;
		}
	}

	switch (msg) {
	case WM_LBUTTONDOWN: {
		DXApplication* dxApp = GetDxAppPtr(hwnd);
		if (dxApp) {
			dxApp->engine_.PlaySE(L"click");

			// コールバック内で buttons が変更されるとイテレータが無効になる問題を解決
			for (auto& btPair : dxApp->buttons) {
				if (btPair.second.AreaCheck(hwnd)) {
					btPair.second.Check(hwnd);
					// 実行後、コンテナが変更された可能性があるためすぐに終了
					break;
				}
			}
		}
		return 0;
	}
					   // ★★★ WM_CTLCOLORSTATIC の処理は不要 (L"STATIC"を排除するため) ★★★
	case WM_DESTROY:
		// メインウィンドウが破棄されたとき、静的HWNDを無効化
		if (hwnd == s_hwnd) {
			s_hwnd = nullptr;
		}
		PostQuitMessage(0);
		return 0;
	default:
		// 無効なhwndが渡された場合、DefWindowProcでクラッシュする可能性があるため、
		// 念のため s_hwnd が有効な場合のみ DefWindowProc を呼ぶか、
		// 最初のチェックで DefWindowProc に任せる。今回は最初のチェックで対応済みとして DefWindowProc を呼ぶ。
		return DefWindowProc(hwnd, msg, wp, lp);
	}
}


// --- AddSelectableText (L"EDIT"コントロールに変更) ---
HWND Win32Application::AddSelectableText(HWND hwndParent,
	int x, int y, int w, int h,
	const std::wstring& text) {

	// s_hInstance が Run() で設定されていることを前提とする
	HINSTANCE hInst = s_hInstance;
	if (hInst == nullptr) {
		hInst = GetModuleHandle(NULL);
	}

	// ★ 修正: L"STATIC" から L"EDIT" に変更し、スクロールと読み取り専用のスタイルを追加
	HWND hText = CreateWindow(
		L"EDIT",        // ウィンドウクラス名
		text.c_str(),     // テキスト内容
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | WS_VSCROLL | ES_AUTOVSCROLL | ES_READONLY | ES_MULTILINE,
		x, y, w, h,
		hwndParent,
		(HMENU)1000,
		hInst,
		nullptr
	);

	if (hText == nullptr) {
		DWORD error = GetLastError();
		std::wstringstream ss;
		ss << L"ERROR: Failed to create EDIT text control. GetLastError() returned: " << error << L"\n";
		OutputDebugString(ss.str().c_str());
	}
	else {
		// Zオーダーを最前面に設定
		SetWindowPos(hText, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		ShowWindow(hText, SW_SHOW);
		UpdateWindow(hText);
	}
	return hText;
}