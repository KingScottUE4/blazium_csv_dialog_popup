/**************************************************************************/
/*  ps3_sysutil.cpp                                                       */
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

#include "ps3_sysutil.h"

#include "core/string/print_string.h"

#ifdef PS3_ENABLED
#include <sysutil/sysutil_msgdialog.h>
#endif

void PS3SystemUtil::_bind_methods() {
	ClassDB::bind_method(D_METHOD("show_message", "message"), &PS3SystemUtil::show_message);
}

void PS3SystemUtil::show_message(const String &p_message) const {
#ifdef PS3_ENABLED
	CharString message = p_message.utf8();
	cellMsgDialogOpen2(CELL_MSGDIALOG_TYPE_SE_TYPE, message.get_data(), nullptr, nullptr, nullptr);
#else
	print_line(vformat("PS3SystemUtil: %s", p_message));
#endif
}

PS3SystemUtil::PS3SystemUtil() {
}
