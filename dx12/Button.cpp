#include "Button.h"
#include "DXApplication.h"

bool Button::AreaChack(HWND hwnd) {
	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(hwnd, &pt);

	RECT clientRect;
	GetClientRect(hwnd, &clientRect);

	// ���݂̃E�B���h�E�T�C�Y�ɍ��킹�Đ�΍��W�ɕϊ�
	float x_abs = x_ratio * clientRect.right;
	float y_abs = y_ratio * clientRect.bottom;
	float w_abs = width_ratio * clientRect.right;
	float h_abs = height_ratio * clientRect.bottom;

	return (pt.x >= x_abs && pt.x <= x_abs + w_abs) &&
		(pt.y >= y_abs && pt.y <= y_abs + h_abs);
}

Button::Button(const std::wstring& key, const std::wstring& imagePath, float x, float y, float width, float height, float windowWidth, float windowHeight, std::function<void()> fn)
	: key(key), imagePath(imagePath), click(fn) {
	// ���K�����W�ɕϊ�
	x_ratio = x / windowWidth;
	y_ratio = y / windowHeight;
	width_ratio = width / windowWidth;
	height_ratio = height / windowHeight;
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