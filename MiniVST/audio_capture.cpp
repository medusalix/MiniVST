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

#include "audio_capture.h"

#include <thread>

#include <Audioclient.h>

AudioCapture::AudioCapture(const std::wstring &name) : AudioDevice(name)
{
	HRESULT result = client->GetService(
		__uuidof(IAudioCaptureClient),
		reinterpret_cast<void**>(&capture)
	);
	checkResult(result);

	result = client->SetEventHandle(event);
	checkResult(result);
}

void AudioCapture::process(Callback callback)
{
	UINT32 framesRead = 0;

	// Read while frames are available
	do
    {
		BYTE *data;
		DWORD flags = 0;

		HRESULT result = capture->GetBuffer(
			&data,
			&framesRead,
			&flags,
			nullptr,
			nullptr
		);
		checkResult(result);

		if (framesRead > 0)
		{
			// Process samples for both channels
			callback(reinterpret_cast<float*>(data), framesRead * 2);
		}

		result = capture->ReleaseBuffer(framesRead);
		checkResult(result);
	} while (framesRead > 0);
}
