/**************************************************************************/
/*  ps3_singleton.cpp                                                     */
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

#include "ps3_singleton.h"

#include "core/config/engine.h"

#ifdef PS3_ENABLED
#include <sys/process.h>
#endif

PlayStation3 *PlayStation3::singleton = nullptr;

PlayStation3 *PlayStation3::get_singleton() {
	return singleton;
}

void PlayStation3::_bind_methods() {
	ClassDB::bind_method(D_METHOD("request_exit"), &PlayStation3::request_exit);
	ClassDB::bind_method(D_METHOD("get_platform_name"), &PlayStation3::get_platform_name);
	ClassDB::bind_method(D_METHOD("get_memory_available"), &PlayStation3::get_memory_available);
}

void PlayStation3::request_exit() {
#ifdef PS3_ENABLED
	sysProcessExit();
#endif
}

String PlayStation3::get_platform_name() const {
	return "PlayStation 3";
}

int PlayStation3::get_memory_available() const {
#ifdef PS3_ENABLED
	
	return 256 * 1024 * 1024;
#else
	return 0;
#endif
}

PlayStation3::PlayStation3() {
	singleton = this;
}

PlayStation3::~PlayStation3() {
	if (singleton == this) {
		singleton = nullptr;
	}
}
