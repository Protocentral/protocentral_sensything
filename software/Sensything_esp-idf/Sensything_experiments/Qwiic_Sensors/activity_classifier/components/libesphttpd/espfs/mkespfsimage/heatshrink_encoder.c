/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

//Stupid wraparound include to make sure object file doesn't end up in heatshrink dir
#ifdef ESPFS_HEATSHRINK
#include "../lib/heatshrink/heatshrink_encoder.c"
#endif
