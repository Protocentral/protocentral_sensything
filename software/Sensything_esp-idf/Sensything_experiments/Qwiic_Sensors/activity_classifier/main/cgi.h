#ifndef CGI_H
#define CGI_H

#include "libesphttpd/httpd.h"

CgiStatus cgiLed(HttpdConnData *connData);
CgiStatus tplLed(HttpdConnData *connData, char *token, void **arg);
CgiStatus tplCounter(HttpdConnData *connData, char *token, void **arg);

#endif
