#pragma once

#include "DXApplication.h" // ★インクルードを追加
#include "Button.h"
#include <vector>

class Win32Application
{
public:
	static void Run(DXApplication* dxApp, HINSTANCE hInstance); // ★引数に追加

private:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
};