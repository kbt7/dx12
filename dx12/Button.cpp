#include "Button.h"
#include "DXApplication.h"
#include "Win32Application.h" // GetDxAppPtrのために必要かもしれません（仮）

bool Button::AreaCheck(HWND hwnd) {
    // マウス位置を取得してクライアント座標に変換
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(hwnd, &pt);

    RECT clientRect;
    GetClientRect(hwnd, &clientRect);

    // 正規化座標 → 絶対座標に変換
    float x_abs = x_ratio * clientRect.right;
    float y_abs = y_ratio * clientRect.bottom;
    float w_abs = width_ratio * clientRect.right;
    float h_abs = height_ratio * clientRect.bottom;

    return (pt.x >= x_abs && pt.x <= x_abs + w_abs) &&
        (pt.y >= y_abs && pt.y <= y_abs + h_abs);
}

// ★★★ 修正箇所: 戻り値を bool に変更し、コールバック実行後に true を返す ★★★
bool Button::Check(HWND hwnd) {
    // 範囲内かつコールバックが設定されている場合のみ実行
    if (AreaCheck(hwnd) && click) {
        click();
        // コールバックが実行されたことを示す
        return true;
    }
    // クリックされなかったか、コールバックが設定されていなかった
    return false;
}