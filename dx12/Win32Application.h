#pragma once

#include "DXApplication.h"
#include "Button.h"
#include "PetalSystem.h"
#include "Game.h"
#include <vector>
#include <execution>
#include <algorithm>
#include <chrono>

class Win32Application
{
public:
	// 静的な変数として、アプリケーションの主要ポインタと状態を管理
	enum GameState { TITLE, PLAY, TAVERN };
	static GameState gs_; // ゲームの状態
	static DXApplication* dxApp_; // DXApplicationポインタ
	static HWND hDetailText_; // 詳細テキストウィンドウハンドル

	static void Run(DXApplication* dxApp, HINSTANCE hInstance);

private:
	static HINSTANCE s_hInstance;
	static HWND s_hwnd;

	static std::function<void()> GoToPlay_;
	static std::function<void(std::function<void()>)> GoToTavern_;

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
	static HWND AddSelectableText(HWND hwndParent,
		int x, int y, int w, int h,
		const std::wstring& text);
};