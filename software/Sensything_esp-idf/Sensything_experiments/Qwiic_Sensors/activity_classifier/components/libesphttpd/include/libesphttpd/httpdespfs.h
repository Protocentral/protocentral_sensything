#ifndef HTTPDESPFS_H
#define HTTPDESPFS_H

#include "httpd.h"

/**
 * The template substitution callback.
 * Returns CGI_MORE if more should be sent within the token, CGI_DONE otherwise.
 */
typedef CgiStatus (* TplCallback)(HttpdConnData *connData, char *token, void **arg);

CgiStatus cgiEspFsHook(HttpdConnData *connData);
CgiStatus ICACHE_FLASH_ATTR cgiEspFsTemplate(HttpdConnData *connData);

/**
 * @return 1 upon success, 0 upon failure
 */
int tplSend(HttpdConnData *conn, const char *str, int len);

#endif
