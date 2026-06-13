/**************************************************************************/
/*  norsx_framebuffer.cpp                                                 */
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

#include "norsx_framebuffer.h"

#include "platform/ps3/ps3_wrapper.h"

#include "core/string/print_string.h"

#ifdef NORSX_ENABLED
#include <NoRSX/NoRSX.h>
#endif

Error NoRSXFramebuffer::initialize() {
	if (initialized) {
		return OK;
	}

	buffer.resize(WIDTH * HEIGHT * 4);
	buffer.fill(0);

#ifdef NORSX_ENABLED
	if (NoRSX_Init(WIDTH, HEIGHT, 32, 0, 0) != 0) {
		ERR_PRINT("NoRSX_Init failed; using internal buffer only.");
	} else {
		print_line("PS3: NoRSX initialized.");
	}
#endif

	initialized = true;
	return OK;
}

void NoRSXFramebuffer::shutdown() {
#ifdef NORSX_ENABLED
	NoRSX_Exit();
#endif
	buffer.clear();
	initialized = false;
}

void NoRSXFramebuffer::clear(uint32_t p_color) {
	const int pixels = WIDTH * HEIGHT;
	uint32_t *ptr = reinterpret_cast<uint32_t *>(buffer.ptrw());
	for (int i = 0; i < pixels; i++) {
		ptr[i] = p_color;
	}
}

void NoRSXFramebuffer::present() {
	if (!initialized) {
		return;
	}

#ifdef NORSX_ENABLED
	NoRSX_SwapBuffers(reinterpret_cast<uint32_t *>(buffer.ptr()));
#else
	
#endif
}

uint8_t *NoRSXFramebuffer::get_buffer() {
	return buffer.ptrw();
}
