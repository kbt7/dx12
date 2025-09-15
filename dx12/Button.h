#pragma once
#include <string>
#include <functional>
#include <windows.h>

class Button {
public:
    Button() = default;
    Button(const std::wstring& key, const std::wstring& imagePath,
        float x, float y, float width, float height,
        float windowWidth, float windowHeight,
        std::function<void()> fn);

    bool AreaCheck(HWND hwnd); // スペル修正
    void Check(HWND hwnd);     // スペル修正

    const std::wstring& getKey() const { return key; }
    const std::wstring& getImagePath() const { return imagePath; }

    float getXAbs(float windowWidth) const { return x_ratio * windowWidth; }
    float getYAbs(float windowHeight) const { return y_ratio * windowHeight; }
    float getWidthAbs(float windowWidth) const { return width_ratio * windowWidth; }
    float getHeightAbs(float windowHeight) const { return height_ratio * windowHeight; }

    void SetCallback(std::function<void()> fn) { click = fn; }

private:
    std::wstring key;
    std::wstring imagePath;
    float x_ratio = 0.0f;
    float y_ratio = 0.0f;
    float width_ratio = 0.0f;
    float height_ratio = 0.0f;
    std::function<void()> click;
};
