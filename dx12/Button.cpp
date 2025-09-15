#include "Button.h"
#include "DXApplication.h"

bool Button::AreaCheck(HWND hwnd) {
    // �}�E�X�ʒu���擾���ăN���C�A���g���W�ɕϊ�
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(hwnd, &pt);

    RECT clientRect;
    GetClientRect(hwnd, &clientRect);

    // ���K�����W �� ��΍��W�ɕϊ�
    float x_abs = x_ratio * clientRect.right;
    float y_abs = y_ratio * clientRect.bottom;
    float w_abs = width_ratio * clientRect.right;
    float h_abs = height_ratio * clientRect.bottom;

    return (pt.x >= x_abs && pt.x <= x_abs + w_abs) &&
        (pt.y >= y_abs && pt.y <= y_abs + h_abs);
}

Button::Button(const std::wstring& key, const std::wstring& imagePath,
    float x, float y, float width, float height,
    float windowWidth, float windowHeight,
    std::function<void()> fn)
    : key(key), imagePath(imagePath), click(fn)
{
    // �E�B���h�E�T�C�Y�ɑ΂��Ĕ䗦�ŕێ�
    x_ratio = x / windowWidth;
    y_ratio = y / windowHeight;
    width_ratio = width / windowWidth;
    height_ratio = height / windowHeight;
}

void Button::Check(HWND hwnd) {
    // �͈͓��Ȃ�R�[���o�b�N���s
    if (AreaCheck(hwnd) && click) {
        click();
    }
}
