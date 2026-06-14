/**************************************************************************/
/*  ps3_sysutil.cpp                                                       */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             BLAZIUM ENGINE                             */
/*                          https://blazium.app                           */
/**************************************************************************/

#include "ps3_sysutil.h"

#include "core/object/class_db.h"
#include "core/string/print_string.h"

#ifdef PS3_ENABLED
#include <sysutil/msg.h>
#endif

void PS3SystemUtil::_bind_methods() {
	ClassDB::bind_method(D_METHOD("show_message", "message"), &PS3SystemUtil::show_message);
	ClassDB::bind_method(D_METHOD("show_message_async", "message"), &PS3SystemUtil::show_message_async);
}

void PS3SystemUtil::show_message(const String &p_message) const {
#ifdef PS3_ENABLED
	CharString message = p_message.utf8();
	msgDialogOpen2(MSG_DIALOG_NORMAL, message.get_data(), nullptr, nullptr, nullptr);
#else
	print_line(vformat("PS3SystemUtil: %s", p_message));
#endif
}

int PS3SystemUtil::show_message_async(const String &p_message) const {
#ifdef PS3_ENABLED
	CharString message = p_message.utf8();
	msgDialogOpen2(MSG_DIALOG_NORMAL, message.get_data(), nullptr, nullptr, nullptr);
	if (msgDialogAbort()) {
		return 2;
	}
	if (msgDialogClose(0.0f)) {
		return 1;
	}
	return 0;
#else
	print_line(vformat("PS3SystemUtil: %s", p_message));
	return 0;
#endif
}

PS3SystemUtil::PS3SystemUtil() {
}
