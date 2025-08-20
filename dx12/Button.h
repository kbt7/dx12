#pragma once
#include <tchar.h>
#include <string>
#include <functional>

class Button {
public:
	Button(const std::wstring& key, float x, float y, float width, float height, std::function<void()> fn);
	void Chack();
	std::wstring getKey();
private:
	const std::wstring& key;
	float x, y, width, height;
	bool visible = false;
	std::function<void()> click;
};