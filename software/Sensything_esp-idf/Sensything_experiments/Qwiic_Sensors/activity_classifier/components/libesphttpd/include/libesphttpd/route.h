#pragma once

#include "cgiredirect.h"

// macros for defining HttpdBuiltInUrl's

/** Route with a CGI handler and two arguments */
#define ROUTE_CGI_ARG2(path, handler, arg1, arg2)  {(path), (handler), (void *)(arg1), (void *)(arg2)}

/** Route with a CGI handler and one arguments */
#define ROUTE_CGI_ARG(path, handler, arg1)         ROUTE_CGI_ARG2((path), (handler), (arg1), NULL)

/** Route with an argument-less CGI handler */
#define ROUTE_CGI(path, handler)                   ROUTE_CGI_ARG2((path), (handler), NULL, NULL)

/** Static file route (file loaded from espfs) */
#define ROUTE_FILE(path, filepath)                 ROUTE_CGI_ARG((path), cgiEspFsHook, (const char*)(filepath))

/** Static file as a template with a replacer function */
#define ROUTE_TPL(path, replacer)                  ROUTE_CGI_ARG((path), cgiEspFsTemplate, (TplCallback)(replacer))

/** Static file as a template with a replacer function, taking additional argument connData->cgiArg2 */
#define ROUTE_TPL_FILE(path, replacer, filepath)   ROUTE_CGI_ARG2((path), cgiEspFsTemplate, (TplCallback)(replacer), (filepath))

/** Redirect to some URL */
#define ROUTE_REDIRECT(path, target)               ROUTE_CGI_ARG((path), cgiRedirect, (const char*)(target))

/** Following routes are basic-auth protected */
#define ROUTE_AUTH(path, passwdFunc)               ROUTE_CGI_ARG((path), authBasic, (AuthGetUserPw)(passwdFunc))

/** Websocket endpoint */
#define ROUTE_WS(path, callback)                   ROUTE_CGI_ARG((path), cgiWebsocket, (WsConnectedCb)(callback))

/** Catch-all filesystem route */
#define ROUTE_FILESYSTEM()                             ROUTE_CGI("*", cgiEspFsHook)

#define ROUTE_END() {NULL, NULL, NULL, NULL}
