/**************************************************************************/
/*  audio_driver_ps3.h                                                    */
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

#pragma once

#include "core/os/mutex.h"
#include "core/os/thread.h"
#include "core/os/thread_safe.h"
#include "servers/audio_server.h"

class AudioDriverPS3 : public AudioDriver {
	Mutex mutex;
	Thread thread;
	ThreadSafeFlag active;
	ThreadSafeFlag exit_thread;
	int mix_rate = 44100;
	int buffer_frames = 1024;
	Vector<int32_t> mix_buffer;

	static void thread_func(void *p_udata);

public:
	virtual const char *get_name() const override { return "PS3"; }

	virtual Error init() override;
	virtual void start() override;
	virtual int get_mix_rate() const override { return mix_rate; }
	virtual int get_mix_buffer_frames() const override { return buffer_frames; }
	virtual SpeakerMode get_speaker_mode() const override { return SPEAKER_MODE_STEREO; }

	virtual void lock() override { mutex.lock(); }
	virtual void unlock() override { mutex.unlock(); }
	virtual void finish() override;
};
