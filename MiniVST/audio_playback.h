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

#include "audio_device.h"

struct IAudioRenderClient;
using UINT32 = unsigned int;

class AudioPlayback : public AudioDevice
{
public:
	AudioPlayback(const std::wstring &name);

private:
	void process(Callback callback) override;

	IAudioRenderClient *render;

	UINT32 bufferSize = 0;
};
