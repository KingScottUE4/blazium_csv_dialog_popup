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
#include <io/pad.h>
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
	if (ioPadGetData(port, &data) == 0) {
		return 0.0f;
	}

	uint16_t pressure = 0;
	switch (p_button) {
		case PAD_CTRL_CROSS:
			pressure = data.PRE_CROSS;
			break;
		case PAD_CTRL_CIRCLE:
			pressure = data.PRE_CIRCLE;
			break;
		case PAD_CTRL_TRIANGLE:
			pressure = data.PRE_TRIANGLE;
			break;
		case PAD_CTRL_SQUARE:
			pressure = data.PRE_SQUARE;
			break;
		case PAD_CTRL_L1:
			pressure = data.PRE_L1;
			break;
		case PAD_CTRL_R1:
			pressure = data.PRE_R1;
			break;
		case PAD_CTRL_L2:
			pressure = data.PRE_L2;
			break;
		case PAD_CTRL_R2:
			pressure = data.PRE_R2;
			break;
		case PAD_CTRL_UP:
			pressure = data.PRE_UP;
			break;
		case PAD_CTRL_DOWN:
			pressure = data.PRE_DOWN;
			break;
		case PAD_CTRL_LEFT:
			pressure = data.PRE_LEFT;
			break;
		case PAD_CTRL_RIGHT:
			pressure = data.PRE_RIGHT;
			break;
		default:
			return 0.0f;
	}

	if (pressure > 0) {
		return pressure / 255.0f;
	}

	const uint16_t digital1 = data.button[PAD_BUTTON_OFFSET_DIGITAL1];
	const uint16_t digital2 = data.button[PAD_BUTTON_OFFSET_DIGITAL2];
	if ((p_button & digital2) != 0 || (p_button & digital1) != 0) {
		return 1.0f;
	}
#endif
	return 0.0f;
}

void PS3Pad::set_rumble(float p_weak, float p_strong) {
#ifdef PS3_ENABLED
	padActParam act = {};
	act.large_motor = (uint8_t)CLAMP(p_strong * 255.0f, 0.0f, 255.0f);
	act.small_motor = p_weak > 0.0f ? 1 : 0;
	ioPadSetActDirect(port, &act);
#endif
}

PS3Pad::PS3Pad() {
}
