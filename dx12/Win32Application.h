#pragma once

#include "DXApplication.h" // ���C���N���[�h��ǉ�
#include "Button.h"
#include <vector>

class Win32Application
{
public:
	static void Run(DXApplication* dxApp, HINSTANCE hInstance); // �������ɒǉ�

private:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
};