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

#include "audio_playback.h"

#include <Audioclient.h>

AudioPlayback::AudioPlayback(const std::wstring &name) : AudioDevice(name)
{
	HRESULT result = client->GetService(
		__uuidof(IAudioRenderClient),
		reinterpret_cast<void**>(&render)
	);
	checkResult(result);

	result = client->SetEventHandle(event);
	checkResult(result);

	result = client->GetBufferSize(&bufferSize);
	checkResult(result);
}

void AudioPlayback::process(Callback callback)
{
	UINT32 available;

	// Read while frames are available
	do
	{
		UINT32 padding = 0;
		HRESULT result = client->GetCurrentPadding(&padding);
		checkResult(result);

		available = bufferSize - padding;

		BYTE *data;
		result = render->GetBuffer(available, &data);
		checkResult(result);

		bool success = false;

		if (available > 0)
		{
			// Process samples for both channels
			success = callback(reinterpret_cast<float*>(data), available * 2);
		}

		// Play silence if audio buffer is empty
		result = render->ReleaseBuffer(
			available,
			success ? 0 : AUDCLNT_BUFFERFLAGS_SILENT
		);
		checkResult(result);
	} while (available > 0);
}
