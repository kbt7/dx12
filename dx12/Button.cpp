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
			nullptr,             // �e�E�B���h�E�̃n���h���i�s�v�Ȃ� nullptr�j
			key.c_str(),        // ���b�Z�[�W�{���iconst wchar_t*�j
			L"debug",       // �^�C�g���iconst wchar_t*�j
			MB_OK | MB_ICONINFORMATION // �{�^����A�C�R���̎��
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