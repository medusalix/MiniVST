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

#include "hotkey.h"
#include "error.h"

#include <algorithm>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

std::map<std::wstring, int> Hotkey::stringKeyMap = {
	{ L"LCONTROL", VK_LCONTROL },
	{ L"RCONTROL", VK_RCONTROL },
	{ L"ALT", VK_MENU },
	{ L"LSHIFT", VK_LSHIFT },
	{ L"RSHIFT", VK_RSHIFT }
};
std::vector<Hotkey*> Hotkey::hotkeys;

Hotkey::Hotkey(Callback callback, const std::vector<std::wstring> &keys)
	: callback(callback)
{
	if (hotkeys.empty())
	{
		initHook();
	}

	initKeys(keys);

	hotkeys.push_back(this);
}

Hotkey::~Hotkey()
{
	auto iterator = std::remove(hotkeys.begin(), hotkeys.end(), this);

	hotkeys.erase(iterator, hotkeys.end());
}

LRESULT CALLBACK Hotkey::keyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode < 0)
    {
        return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }
    
	KBDLLHOOKSTRUCT *key = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
	bool down = wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN;

	for (Hotkey *hotkey : hotkeys)
	{ 
		auto iterator = hotkey->keysDown.find(key->vkCode);

		// Check if key is in array
		if (iterator != hotkey->keysDown.end())
		{
			iterator->second = down;

			bool pressed = std::all_of(
				hotkey->keysDown.begin(),
				hotkey->keysDown.end(),
				[](std::pair<int, bool> entry) {return entry.second; }
			);

			// If hotkey pressed changes
			if (pressed != hotkey->pressed)
			{
				hotkey->pressed = pressed;
				hotkey->callback(pressed);
			}
		}
	}

	return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

void Hotkey::initHook()
{
	HHOOK hook = SetWindowsHookEx(
		WH_KEYBOARD_LL,
		keyboardProc,
		nullptr,
		0
	);

	if (hook == nullptr)
	{
		throw Error(L"Error initializing keyboard hook");
	}
}

void Hotkey::initKeys(const std::vector<std::wstring> &keys)
{
	for (std::wstring key : keys)
	{
		std::transform(key.begin(), key.end(), key.begin(), toupper);

		// Key is a single letter
		if (key.length() == 1)
		{
			wchar_t c = key[0];

			if (c >= '0' && c <= 'Z')
			{
				keysDown[c] = false;
			}
		}

		else
		{
			try
			{
				int vKey = stringKeyMap.at(key);

				keysDown[vKey] = false;
			}

			catch (std::out_of_range&)
			{
				throw Error(L"No key for name found: " + key);
			}
		}
	}
}
