/*
 * Copyright (C) 2018 Medusalix
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <string>

struct HINSTANCE__;
struct HWND__;

using HINSTANCE = HINSTANCE__*;
using HWND = HWND__*;

using LRESULT = long;
using UINT = unsigned int;
using LONG = long;
using WPARAM = unsigned int;
using LPARAM = long;
using WNDPROC = LRESULT (__stdcall*)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
using RECT = struct tagRECT;

class Window
{
public:
	Window(HINSTANCE instance, WNDPROC windowProc);

	void create();
	void show();

	void setClassName(const std::wstring &className);
	void setTitle(const std::wstring &title);
	void setIcon(int icon);
	void setSize(int width, int height);

	HWND getHandle() const;

private:
	HINSTANCE instance;
	WNDPROC windowProc;
	HWND handle = nullptr;

	std::wstring className;
	std::wstring title;
	int icon = 0;

	int width = 0, height = 0;

	void initWindowClass();
	RECT calcDimensions();
};
