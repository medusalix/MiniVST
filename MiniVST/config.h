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
#include <vector>
#include <map>

class Config
{
public:
	Config(const std::wstring &path);

	std::wstring getString(const std::wstring &key);
	std::vector<std::wstring> getStrings(
		const std::wstring &key,
		const std::wstring &separator
	);
	int getInt(const std::wstring &key);
	bool getBool(const std::wstring &key);

private:
	std::map<std::wstring, std::wstring> config;
};
