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

#include "audio_device.h"
#include "error.h"

#include <thread>

#include <comdef.h>
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <Functiondiscoverykeys_devpkey.h>

std::map<std::wstring, IMMDevice*> AudioDevice::devices;

AudioDevice::AudioDevice(const std::wstring &name)
{
	if (devices.empty())
	{
		enumDevices();
	}

	try
	{
		IMMDevice *device = devices.at(name);
		initAudioClient(device);
	}

	catch (std::out_of_range&)
	{
		throw Error(L"Couldn't find audio device with name: " + name);
	}

	event = CreateEvent(nullptr, false, false, nullptr);

	if (event == nullptr)
	{
		throw Error(L"Error creating audio event");
	}
}

void AudioDevice::start(Callback callback)
{
	HRESULT result = client->Start();
	checkResult(result);

	std::thread([this, callback]()
	{
		while (true)
		{
			WaitForSingleObject(event, INFINITE);

			process(callback);
		}
	}).detach();
}

int AudioDevice::getSampleRate() const
{
	return sampleRate;
}

void AudioDevice::checkResult(HRESULT result)
{
	if (FAILED(result))
	{
		_com_error error(result);

		throw Error(L"Error in COM function: " + std::wstring(error.ErrorMessage()));
	}
}

void AudioDevice::enumDevices()
{
	HRESULT result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	checkResult(result);

	IMMDeviceEnumerator *enumerator;
	result = CoCreateInstance(
		__uuidof(MMDeviceEnumerator),
		nullptr,
		CLSCTX_ALL,
		__uuidof(IMMDeviceEnumerator),
		reinterpret_cast<void**>(&enumerator)
	);
	checkResult(result);

	IMMDeviceCollection *collection;
	result = enumerator->EnumAudioEndpoints(
		eAll,
		DEVICE_STATE_ACTIVE,
		&collection
	);
	checkResult(result);

	UINT count = 0;
	result = collection->GetCount(&count);
	checkResult(result);

	for (UINT i = 0; i < count; i++)
	{
		IMMDevice *device;
		result = collection->Item(i, &device);
		checkResult(result);

		IPropertyStore *propertyStore;
		result = device->OpenPropertyStore(STGM_READ, &propertyStore);
		checkResult(result);

		PROPVARIANT property = {};
		result = propertyStore->GetValue(PKEY_Device_FriendlyName, &property);
		checkResult(result);

		devices[property.pwszVal] = device;
	}
}

void AudioDevice::initAudioClient(IMMDevice *device)
{
	HRESULT result = device->Activate(
		__uuidof(IAudioClient),
		CLSCTX_ALL,
		nullptr,
		reinterpret_cast<void**>(&client)
	);
	checkResult(result);

	WAVEFORMATEX *format;
	result = client->GetMixFormat(&format);
	checkResult(result);

	if (format->wFormatTag != WAVE_FORMAT_EXTENSIBLE)
	{
		throw Error(L"Invalid format for audio device: " + std::to_wstring(format->wFormatTag));
	}

	if (format->nChannels != 2)
	{
		throw Error(L"Audio device must be stereo");
	}

	sampleRate = format->nSamplesPerSec;

	result = client->Initialize(
		AUDCLNT_SHAREMODE_SHARED,
		AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
		0,
		0,
		format,
		nullptr
	);
	checkResult(result);
}
