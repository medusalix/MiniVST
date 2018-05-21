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

#include "vst_plugin.h"
#include "error.h"

#include <fstream>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#pragma warning(disable : 4996)
#include <aeffect.h>

using VstMain = AEffect* (*)(audioMasterCallback callback);

VstPlugin::VstPlugin(const std::wstring &path)
{
	HMODULE library = LoadLibrary(path.c_str());

	if (library == nullptr)
	{
		throw Error(L"Error loading plugin: " + path);
	}

	FARPROC mainAddress = GetProcAddress(library, "VSTPluginMain");

	if (mainAddress == nullptr)
	{
		mainAddress = GetProcAddress(library, "main");
	}

	if (mainAddress == nullptr)
	{
		throw Error(L"Plugin entry point not found");
	}

	effect = reinterpret_cast<VstMain>(mainAddress)(hostCallback);

	if (effect == nullptr)
	{
		throw Error(L"Plugin initialization failed");
	}

	if (effect->magic != kEffectMagic)
	{
		throw Error(L"Plugin magic number mismatch");
	}

	if (effect->numOutputs > 2)
	{
		throw Error(L"Plugin has more than 2 output channels");
	}

	// Find plugin name
	size_t fileIndex = path.find_last_of(L'/');

	if (fileIndex == std::wstring::npos)
	{
		fileIndex = path.find_last_of(L'\\');
	}

	size_t extIndex = path.find_last_of(L'.');

	name = path.substr(fileIndex + 1, extIndex - fileIndex - 1);
}

std::wstring VstPlugin::getName() const
{
	return name;
}

void VstPlugin::loadSettings()
{
	std::ifstream file(name + L".prg", std::ios::binary);

	if (!file)
	{
		return;
	}

	int id = 0;
	file.read(reinterpret_cast<char*>(&id), sizeof(int));

	// Don't load chunk file from different plugin
	if (id != effect->uniqueID)
	{
		return;
	}

	int chunkSize = 0;
	file.read(reinterpret_cast<char*>(&chunkSize), sizeof(int));

	if (chunkSize == 0)
	{
		return;
	}

	std::vector<char> chunk;
	chunk.resize(chunkSize);
	
	file.read(chunk.data(), chunkSize);

	effect->dispatcher(effect, effSetChunk, 0, chunkSize, chunk.data(), 0);
}

void VstPlugin::saveSettings()
{
	std::ofstream file(name + L".prg", std::ios::binary);

	if (!file)
	{
		return;
	}

	char *chunk;
	int chunkSize = effect->dispatcher(effect, effGetChunk, 0, 0, &chunk, 0);

	if (chunkSize == 0)
	{
		return;
	}

	// Write unique id for identification
	file.write(reinterpret_cast<char*>(&effect->uniqueID), sizeof(int));
	file.write(reinterpret_cast<char*>(&chunkSize), sizeof(int));
	file.write(chunk, chunkSize);
}

void VstPlugin::start(int sampleRate)
{
	// Block size is sample rate [kHz] * channels
	int blockSize = sampleRate / 100 * 2;

	input.emplace_back(blockSize);
	input.emplace_back(blockSize);
	output.emplace_back(blockSize);
	output.emplace_back(blockSize);

	rawInput[0] = input[0].data();
	rawInput[1] = input[1].data();
	rawOutput[0] = output[0].data();
	rawOutput[1] = output[1].data();

	effect->dispatcher(effect, effOpen, 0, 0, nullptr, 0);
	effect->dispatcher(effect, effSetSampleRate, 0, 0, nullptr, static_cast<float>(sampleRate));
	effect->dispatcher(effect, effSetBlockSize, 0, blockSize, nullptr, 0);
	effect->dispatcher(effect, effMainsChanged, 0, 1, nullptr, 0);
}

void VstPlugin::getEditorRect(int &width, int &height)
{
	ERect **rect = new ERect*;
	effect->dispatcher(effect, effEditGetRect, 0, 0, rect, 0);

	width = (*rect)->right - (*rect)->left;
	height = (*rect)->bottom - (*rect)->top;

	delete rect;
}

void VstPlugin::openEditor(void *windowHandle)
{
	effect->dispatcher(effect, effEditOpen, 0, 0, windowHandle, 0);
}

void VstPlugin::process(float samples[], int count)
{
	// Samples for each channel
	int channelSamples = count / 2;

	// Move both channels into separate arrays
	for (int i = 0; i < channelSamples; i++)
	{
		input[0][i] = samples[i * 2];
		input[1][i] = samples[i * 2 + 1];
	}

	if (effect->flags & effFlagsCanReplacing)
	{
		effect->processReplacing(effect, rawInput, rawOutput, channelSamples);
	}

	else
	{
		effect->__processDeprecated(effect, rawInput, rawOutput, channelSamples);
	}

	// Move output back into one array
	for (int i = 0; i < channelSamples; i++)
	{
		samples[i * 2] = output[0][i];
		samples[i * 2 + 1] = output[1][i];
	}
}

int VstPlugin::hostCallback(
	AEffect *effect,
	int opcode,
	int index,
	int value,
	void *ptr,
	float opt
)
{
	switch (opcode)
	{
		case audioMasterVersion:
			// VST 2.4
			return 2400;

		default:
			return 0;
	}
}
