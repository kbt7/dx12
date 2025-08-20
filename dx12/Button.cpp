#include "Button.h"

Button::Button(const std::wstring& key, float x, float y, float width, float height, std::function<void()> fn)
	: key(key), x(x), y(y), width(width), height(height), click(fn) {
}

void Button::Chack() {
	click();
}

std::wstring Button::getKey() {
	return key;
}