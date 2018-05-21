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

#include "window.h"
#include "error.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

Window::Window(HINSTANCE instance, WNDPROC windowProc)
	: instance(instance), windowProc(windowProc) {}

void Window::create()
{
	initWindowClass();
	RECT rect = calcDimensions();

	handle = CreateWindow(
		className.c_str(),
		title.c_str(),
		WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rect.right - rect.left,
		rect.bottom - rect.top,
		nullptr,
		nullptr,
		instance,
		nullptr
	);

	if (handle == nullptr)
	{
		throw Error(L"Error creating window");
	}
}

void Window::show()
{
	ShowWindow(handle, SW_SHOW);

	MSG message;

	while (GetMessage(&message, nullptr, 0, 0) != 0)
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
}

void Window::setClassName(const std::wstring &className)
{
	this->className = className;
}

void Window::setTitle(const std::wstring &title)
{
	if (handle == nullptr)
	{
		this->title = title;
	}

	else
	{
		SetWindowText(handle, title.c_str());
	}
}

void Window::setIcon(int icon)
{
	this->icon = icon;
}

void Window::setSize(int width, int height)
{
	this->width = width;
	this->height = height;
}

HWND Window::getHandle() const
{
	return handle;
}

void Window::initWindowClass()
{
	HANDLE iconHandle = LoadImage(
		instance,
		MAKEINTRESOURCE(icon),
		IMAGE_ICON,
		0,
		0,
		LR_DEFAULTSIZE | LR_SHARED
	);

	if (iconHandle == nullptr)
	{
		throw Error(L"Error loading window icon");
	}

	WNDCLASSEX windowClass = {};

	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.lpfnWndProc = windowProc;
	windowClass.hInstance = instance;
	windowClass.hIcon = static_cast<HICON>(iconHandle);
	windowClass.lpszClassName = className.c_str();

	if (!RegisterClassEx(&windowClass))
	{
		throw Error(L"Error registering window class");
	}
}

RECT Window::calcDimensions()
{
	RECT rect = {};

	rect.right = width;
	rect.bottom = height;

	if (!AdjustWindowRect(&rect, WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX, false))
	{
		throw Error(L"Error calculating window size");
	}

	return rect;
}
