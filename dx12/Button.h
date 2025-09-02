#pragma once
#include <tchar.h>
#include <string>
#include <functional>
#include <windows.h>
#include <math.h>

class Button {
public:
	Button() {};
	Button(const std::wstring& key, const std::wstring& imagePass, float x, float y, float width, float height, std::function<void()> fn);
	void Chack(HWND hwnd);
	void OnCursol(HWND hwnd);
	std::wstring getKey();
	std::wstring getImagePass();
	float getX();
	float getY();
	float getWidth();
	float getHeight();
private:
	std::wstring key;
	std::wstring imagePass;
	float x, y, width, height;
	bool visible = false;
	std::function<void()> click;
	bool AreaChack(HWND hwnd);
};