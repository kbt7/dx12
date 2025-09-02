#include "Button.h"
#include "DXApplication.h"

bool Button::AreaChack(HWND hwnd) {
	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(hwnd, &pt);

	return (x < pt.x && pt.x < x + width) && (y < pt.y && pt.y < y + height);
}

Button::Button(const std::wstring& key, const std::wstring& imagePass, float x, float y, float width, float height, std::function<void()> fn)
	: key(key), imagePass(imagePass), x(x), y(y), width(width), height(height), click(fn) {
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

std::wstring Button::getKey() {
	return key;
}

std::wstring Button::getImagePass() {
	return imagePass;
}

float Button::getX() {
	return x;
}

float Button::getY() {
	return y;
}

float Button::getWidth() {
	return width;
}

float Button::getHeight() {
	return height;
}