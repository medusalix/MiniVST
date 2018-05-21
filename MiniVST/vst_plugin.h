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

struct AEffect;

class VstPlugin
{
public:
	VstPlugin(const std::wstring &path);

	std::wstring getName() const;

	void loadSettings();
	void saveSettings();

	void start(int sampleRate);

	void getEditorRect(int &width, int &height);
	void openEditor(void *windowHandle);

	void process(float samples[], int count);

private:
	AEffect *effect;
	std::wstring name;

	std::vector<std::vector<float>> input, output;
	float *rawInput[2], *rawOutput[2];

	static int hostCallback(
		AEffect *effect,
		int opcode,
		int index,
		int value,
		void *ptr,
		float opt
	);
};
