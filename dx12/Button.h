#pragma once
#include <tchar.h>
#include <string>
#include <functional>
#include <windows.h>
#include <math.h>

class Button {
public:
	Button() {};
	Button(const std::wstring& key, const std::wstring& imagePath, float x, float y, float width, float height, float windowWidth, float windowHeight, std::function<void()> fn);
	void Chack(HWND hwnd);
	void OnCursol(HWND hwnd);
	float getXAbs(float windowWidth) const { return x_ratio * windowWidth; }
	float getYAbs(float windowHeight) const { return y_ratio * windowHeight; }
	float getWidthAbs(float windowWidth) const { return width_ratio * windowWidth; }
	float getHeightAbs(float windowHeight) const { return height_ratio * windowHeight; }
	const std::wstring& getKey() const { return key; }
	const std::wstring& getImagePath() const { return imagePath; }
	bool AreaChack(HWND hwnd);
private:
	float x_ratio;      // ウィンドウ幅に対する比率 (0～1)
	float y_ratio;      // ウィンドウ高さに対する比率
	float width_ratio;
	float height_ratio;
	std::wstring key;
	std::wstring imagePath;
	bool visible = false;
	std::function<void()> click;
};