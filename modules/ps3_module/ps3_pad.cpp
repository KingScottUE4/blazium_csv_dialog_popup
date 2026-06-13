/**************************************************************************/
/*  ps3_pad.cpp                                                           */
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

#include "ps3_pad.h"

#include "core/math/math_funcs.h"

#ifdef PS3_ENABLED
#include <pad.h>
#endif

void PS3Pad::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_port", "port"), &PS3Pad::set_port);
	ClassDB::bind_method(D_METHOD("get_port"), &PS3Pad::get_port);
	ClassDB::bind_method(D_METHOD("get_pressure", "button"), &PS3Pad::get_pressure);
	ClassDB::bind_method(D_METHOD("set_rumble", "weak", "strong"), &PS3Pad::set_rumble);
}

void PS3Pad::set_port(int p_port) {
	port = p_port;
}

int PS3Pad::get_port() const {
	return port;
}

float PS3Pad::get_pressure(int p_button) const {
#ifdef PS3_ENABLED
	padData data;
	if (padGetData(port, &data) == 0) {
		return 0.0f;
	}
	if (p_button >= 0 && p_button < 32) {
		return data.button[p_button] ? 1.0f : 0.0f;
	}
#endif
	return 0.0f;
}

void PS3Pad::set_rumble(float p_weak, float p_strong) {
#ifdef PS3_ENABLED
	const uint8_t large = (uint8_t)CLAMP(p_strong * 255.0f, 0.0f, 255.0f);
	const uint8_t small = (uint8_t)CLAMP(p_weak * 255.0f, 0.0f, 255.0f);
	padSetActDirect(port, large, small);
#endif
}

PS3Pad::PS3Pad() {
}
