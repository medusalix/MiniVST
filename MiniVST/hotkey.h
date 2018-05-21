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

#include <vector>
#include <map>

using LRESULT = long;
using WPARAM = unsigned int;
using LPARAM = long;

class Hotkey
{
public:
	using Callback = void (*)(bool pressed);

	Hotkey(Callback callback, const std::vector<std::wstring> &keys);
	~Hotkey();
private:
	static LRESULT __stdcall keyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

	void initHook();
	void initKeys(const std::vector<std::wstring> &keys);

	static std::vector<Hotkey*> hotkeys;
	static std::map<std::wstring, int> stringKeyMap;

	Callback callback;
	std::map<int, bool> keysDown;
	bool pressed = false;
};
