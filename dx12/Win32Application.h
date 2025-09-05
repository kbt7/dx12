#pragma once

#include "DXApplication.h" // ���C���N���[�h��ǉ�
#include "Button.h"
#include "PetalSystem.h"
#include <vector>
#include <execution> // ����A���S���Y��
#include <algorithm> // std::for_each
#include <chrono>

class Win32Application
{
public:
	static void Run(DXApplication* dxApp, HINSTANCE hInstance); // �������ɒǉ�

private:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
	static HWND AddSelectableText(HWND hwndParent, HINSTANCE hInstance,
		int x, int y, int w, int h,
		const std::wstring& text);
};