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

#include <memory>
#include <mutex>

class AudioBuffer
{
public:
	AudioBuffer(size_t size);

	bool push(float samples[], size_t count);
	bool pop(float samples[], size_t count);

private:
	std::unique_ptr<float[]> buffer;

	size_t size;
	size_t head = 0, tail = 0;

	std::mutex mutex;
};
