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

#include "error.h"
#include "config.h"
#include "audio_capture.h"
#include "audio_playback.h"
#include "audio_buffer.h"
#include "vst_plugin.h"
#include "window.h"
#include "hotkey.h"

#include "resource.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

const std::wstring ConfigPath = L"MiniVST.cfg";
const std::wstring WindowClass = L"MiniVST";
const std::wstring WindowTitle = L"MiniVST v0.0.1 ©Severin v. W.";

bool hotkeyHold;
bool vstActive;

AudioCapture *capture;
AudioPlayback *playback;
VstPlugin *vst;
Window *window;
AudioBuffer *buffer;

LRESULT CALLBACK windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

std::wstring getWindowTitle()
{
	std::wstring state = vstActive ? L"Active" : L"Inactive";

	return WindowTitle + L" | " + vst->getName() + L" | " + state;
}

bool captureCallback(float samples[], int count)
{
	if (vstActive)
	{
		vst->process(samples, count);
	}

	return buffer->push(samples, count);
}

bool playbackCallback(float samples[], int count)
{
	return buffer->pop(samples, count);
}

void hotkeyCallback(bool pressed)
{
	if (hotkeyHold)
	{
		vstActive = pressed;
	}

	else if (pressed)
	{
		vstActive = !vstActive;
	}

	window->setTitle(getWindowTitle());
}

int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow
)
{
	try
	{
		Config config(ConfigPath);

		capture = new AudioCapture(config.getString(L"input_device"));
		playback = new AudioPlayback(config.getString(L"output_device"));
		
		if (capture->getSampleRate() != playback->getSampleRate())
		{
			throw Error(L"Sample rate of input and output device doesn't match");
		}

		buffer = new AudioBuffer(config.getInt(L"buffer_size"));

		vst = new VstPlugin(config.getString(L"plugin_path"));
		vst->start(capture->getSampleRate());
		vst->loadSettings();

		int width, height;

		vst->getEditorRect(width, height);

		window = new Window(hInstance, windowProc);

		window->setClassName(WindowClass);
		window->setTitle(getWindowTitle());
		window->setIcon(IDI_ICON);
		window->setSize(width, height);
		window->create();

		vst->openEditor(window->getHandle());

		hotkeyHold = config.getBool(L"hotkey_hold");

		Hotkey hotkey(hotkeyCallback, config.getStrings(L"hotkey", L" + "));

		capture->start(captureCallback);
		playback->start(playbackCallback);

		window->show();

		if (config.getBool(L"save_settings"))
		{
			vst->saveSettings();
		}
	}

	catch (Error &ex)
	{
		MessageBox(nullptr, ex.getMessage().c_str(), L"MiniVST Error", MB_ICONERROR);

		return -1;
	}

	return 0;
}
