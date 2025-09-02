#include "Button.h"
#include "DXApplication.h"

bool Button::AreaChack(HWND hwnd) {
	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(hwnd, &pt);

	RECT clientRect;
	GetClientRect(hwnd, &clientRect);

	// 現在のウィンドウサイズに合わせて絶対座標に変換
	float x_abs = x_ratio * clientRect.right;
	float y_abs = y_ratio * clientRect.bottom;
	float w_abs = width_ratio * clientRect.right;
	float h_abs = height_ratio * clientRect.bottom;

	return (pt.x >= x_abs && pt.x <= x_abs + w_abs) &&
		(pt.y >= y_abs && pt.y <= y_abs + h_abs);
}

Button::Button(const std::wstring& key, const std::wstring& imagePath, float x, float y, float width, float height, float windowWidth, float windowHeight, std::function<void()> fn)
	: key(key), imagePath(imagePath), click(fn) {
	// 正規化座標に変換
	x_ratio = x / windowWidth;
	y_ratio = y / windowHeight;
	width_ratio = width / windowWidth;
	height_ratio = height / windowHeight;
}

void Button::Chack(HWND hwnd) {

	if (AreaChack(hwnd)) {
		click();
		MessageBoxW(
			nullptr,             // 親ウィンドウのハンドル（不要なら nullptr）
			key.c_str(),        // メッセージ本文（const wchar_t*）
			L"debug",       // タイトル（const wchar_t*）
			MB_OK | MB_ICONINFORMATION // ボタンやアイコンの種類
		);
		
	}
}