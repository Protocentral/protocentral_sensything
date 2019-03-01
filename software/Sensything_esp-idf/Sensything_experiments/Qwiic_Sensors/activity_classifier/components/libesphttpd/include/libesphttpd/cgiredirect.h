#pragma once

#include "httpd.h"

CgiStatus cgiRedirect(HttpdConnData *connData);

// This CGI function redirects to a fixed url of http://[hostname]/ if hostname
// field of request isn't already that hostname. Use this in combination with
// a DNS server that redirects everything to the ESP in order to load a HTML
// page as soon as a phone, tablet etc connects to the ESP.
//
// NOTE: If your httpd server is listening on all interfaces this will also
//       redirect connections when the ESP is in STA mode, potentially to a
//       hostname that is not in the 'official' DNS and so will fail.
//
// It is recommended to place this cgi route early or first in your route list
// so the redirect occurs before other route processing
//
// The 'cgiArg' to this cgi function is the hostname that the client will be redirected
// to.
//
// If the hostname matches the hostname specified in cgiArg then this cgi function
// will return HTTPD_CGI_NOTFOUND, causing the libesphttpd server to skip
// over this cgi function and to continue processing with the next route
//
// Example usage:
//         ROUTE_CGI_ARG("*", cgiRedirectToHostname, "HOSTNAME_HERE"),
CgiStatus cgiRedirectToHostname(HttpdConnData *connData);

CgiStatus cgiRedirectApClientToHostname(HttpdConnData *connData);
