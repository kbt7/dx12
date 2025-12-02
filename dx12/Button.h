#pragma once
#include <string>
#include <functional>
#include <windows.h>

class Button {
public:
    Button() = default;
    Button(const std::wstring& k, const std::wstring& img,
        float x, float y, float w, float h,
        float windowW, float windowH,
        std::function<void()> fn)
        : key(k), imagePath(img),
        x_ratio(x / windowW), y_ratio(y / windowH),
        width_ratio(w / windowW), height_ratio(h / windowH),
        click(fn) {}

    bool AreaCheck(HWND hwnd);
    bool Check(HWND hwnd);
    void SetCallback(std::function<void()> fn) { click = fn; }
    bool HasCallback() const { return click != nullptr; }

    const std::wstring& getKey() const { return key; }
    const std::wstring& getImagePath() const { return imagePath; }
    float getXAbs(float windowWidth) const { return x_ratio * windowWidth; }
    float getYAbs(float windowHeight) const { return y_ratio * windowHeight; }
    float getWidthAbs(float windowWidth) const { return width_ratio * windowWidth; }
    float getHeightAbs(float windowHeight) const { return height_ratio * windowHeight; }

private:
    std::wstring key;
    std::wstring imagePath;
    float x_ratio = 0.0f;
    float y_ratio = 0.0f;
    float width_ratio = 0.0f;
    float height_ratio = 0.0f;
    std::function<void()> click;
};
