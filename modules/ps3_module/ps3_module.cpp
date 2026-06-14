/**************************************************************************/
/*  ps3_module.cpp                                                        */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             BLAZIUM ENGINE                             */
/*                          https://blazium.app                           */
/**************************************************************************/

#include "ps3_module.h"

#include "core/object/class_db.h"

#ifdef PS3_ENABLED
#include <sysutil/sysutil.h>
#endif

void PS3Module::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_sdk_version"), &PS3Module::get_sdk_version);
	ClassDB::bind_method(D_METHOD("get_firmware_version"), &PS3Module::get_firmware_version);
}

String PS3Module::get_sdk_version() const {
#ifdef PS3_ENABLED
	return "PSL1GHT";
#else
	return "PSL1GHT";
#endif
}

String PS3Module::get_firmware_version() const {
#ifdef PS3_ENABLED
	char version[64] = {};
	if (sysUtilGetSystemParamString(0x101, version, sizeof(version)) == 0) {
		return String(version);
	}
#endif
	return "PS3";
}
