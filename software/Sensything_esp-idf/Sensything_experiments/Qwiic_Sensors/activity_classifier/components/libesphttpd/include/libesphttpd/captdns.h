#pragma once

// NOTE: hostnames that end in '.local' appear to conflict with mDNS resolution,
//       at least in MacOS. These hostnames will not be redirected to the captive
//       dns server.
void captdnsInit(void);
