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

#include "audio_buffer.h"

#include <algorithm>

AudioBuffer::AudioBuffer(size_t size) : buffer(new float[size]), size(size) {}

bool AudioBuffer::push(float samples[], size_t count)
{
	std::lock_guard<std::mutex> guard(mutex);

	for (size_t i = 0; i < count; i++)
	{
		buffer[head] = samples[i];
		head = (head + 1) % size;

		// Overwrite if buffer full
		if (head == tail)
		{
			tail = (tail + 1) % size;
		}
	}

	return true;
}

bool AudioBuffer::pop(float samples[], size_t count)
{
	std::lock_guard<std::mutex> guard(mutex);

	for (size_t i = 0; i < count; i++)
	{
		// Buffer is empty
		if (head == tail)
		{
			return false;
		}

		samples[i] = buffer[tail];
		tail = (tail + 1) % size;
	}

	return true;
}
