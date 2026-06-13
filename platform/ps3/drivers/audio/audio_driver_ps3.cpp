/**************************************************************************/
/*  audio_driver_ps3.cpp                                                  */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             BLAZIUM ENGINE                             */
/*                          https://blazium.app                           */
/**************************************************************************/
/* Copyright (c) 2024-present Blazium Engine contributors.                */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "audio_driver_ps3.h"

#include "core/os/os.h"
#include "core/string/print_string.h"

#ifdef PS3SOUND_ENABLED
#include <ps3sound/sound.h>
#endif

void AudioDriverPS3::thread_func(void *p_udata) {
	AudioDriverPS3 *driver = static_cast<AudioDriverPS3 *>(p_udata);
	const uint64_t usdelay = (driver->buffer_frames / float(driver->mix_rate)) * 1000000;

	while (!driver->exit_thread.is_set()) {
		if (driver->active.is_set()) {
			driver->lock();
			driver->audio_server_process(driver->buffer_frames, driver->mix_buffer.ptrw());
			driver->unlock();

#ifdef PS3SOUND_ENABLED
			soundPlay(driver->mix_buffer.ptr(), driver->buffer_frames * 2);
#endif
		}

		OS::get_singleton()->delay_usec(usdelay);
	}
}

Error AudioDriverPS3::init() {
	mix_rate = 44100;
	buffer_frames = 1024;
	mix_buffer.resize(buffer_frames * 2);
	active.clear();
	exit_thread.clear();

#ifdef PS3SOUND_ENABLED
	if (soundInit() != 0) {
		ERR_PRINT("PS3: soundInit failed.");
		return FAILED;
	}
	print_line("PS3: ps3soundlib initialized.");
#endif

	thread.start(thread_func, this);
	return OK;
}

void AudioDriverPS3::start() {
	active.set();
}

void AudioDriverPS3::finish() {
	exit_thread.set();
	if (thread.is_started()) {
		thread.wait_to_finish();
	}

#ifdef PS3SOUND_ENABLED
	soundQuit();
#endif
}
