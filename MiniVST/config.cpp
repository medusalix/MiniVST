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

#include "config.h"
#include "error.h"

#include <fstream>
#include <codecvt>

Config::Config(const std::wstring &path)
{
	std::wifstream file(path);

	if (!file)
	{
		throw Error(L"Config file not found: " + path);
	}

	// Enable UTF-8 for file reading
	file.imbue(std::locale(file.getloc(), new std::codecvt_utf8<wchar_t>));

	// Skip whitespace on first line
	file >> std::ws;

	std::wstring line;

	while (std::getline(file, line))
	{
		// Check if line is empty or comment
		if (line.empty() || line[0] == L'#')
		{
			continue;
		}

		size_t index = line.find(L" = ");

		if (index == std::wstring::npos)
		{
			throw Error(L"Incorrectly formatted config file");
		}

		std::wstring key = line.substr(0, index);
		std::wstring value = line.substr(index + 3);

		config[key] = value;
	}
}

std::wstring Config::getString(const std::wstring &key)
{
	try
	{
		return config.at(key);
	}

	catch (std::out_of_range&)
	{
		throw Error(L"Config entry not found: " + key);
	}
}

std::vector<std::wstring> Config::getStrings(
	const std::wstring &key,
	const std::wstring &separator
)
{
	std::wstring value = getString(key);
	std::vector<std::wstring> parts;

	size_t offset = 0;

	while (true)
	{
		size_t index = value.find(separator, offset);

		if (index == std::wstring::npos)
		{
			break;
		}

		parts.push_back(value.substr(offset, index - offset));
		offset = index + 3;
	}

	parts.push_back(value.substr(offset));

	return parts;
}

int Config::getInt(const std::wstring &key)
{
	std::wstring value = getString(key);

	try
	{
		return std::stoi(value);
	}

	catch (std::logic_error&)
	{
		throw Error(L"Invalid integer value for key: " + key);
	}
}

bool Config::getBool(const std::wstring &key)
{
	std::wstring value = getString(key);

	if (value == L"true")
	{
		return true;
	}

	if (value == L"false")
	{
		return false;
	}

	throw Error(L"Invalid boolean value for key: " + key);
}
