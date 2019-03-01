/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "libesphttpd/espfs.h"
#ifdef ESPFS_HEATSHRINK
//Stupid wrapper so we don't have to move c-files around
//Also loads httpd-specific config.

#ifdef __ets__
//esp build

#include <libesphttpd/esp.h>

#endif

#include "heatshrink_config_custom.h"
#include "../lib/heatshrink/heatshrink_decoder.c"


#endif
