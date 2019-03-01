/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
Platform-dependent routines, FreeRTOS version


Thanks to my collague at Espressif for writing the foundations of this code.
*/

/* Copyright 2017 Jeroen Domburg <git@j0h.nl> */
/* Copyright 2017 Chris Morgan <chmorgan@gmail.com> */

#if defined(linux) || defined(FREERTOS)

#ifdef linux
#include <libesphttpd/linux.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>

#else
#include <libesphttpd/esp.h>
#endif

#include "libesphttpd/httpd.h"
#include "libesphttpd/platform.h"
#include "httpd-platform.h"
#include "libesphttpd/httpd-freertos.h"

#include "esp_log.h"

#ifdef FREERTOS
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#endif

#define fr_of_instance(instance) esp_container_of(instance, HttpdFreertosInstance, httpdInstance)
#define frconn_of_conn(conn) esp_container_of(conn, RtosConnType, connData)


const static char* TAG = "httpd-freertos";


int ICACHE_FLASH_ATTR httpdPlatSendData(HttpdInstance *pInstance, HttpdConnData *pConn, char *buff, int len) {
    int bytesWritten;
#ifdef CONFIG_ESPHTTPD_SSL_SUPPORT
    HttpdFreertosInstance *pFR = fr_of_instance(pInstance);
#endif
    RtosConnType *pRconn = frconn_of_conn(pConn);
    pRconn->needWriteDoneNotif=1;

#ifdef CONFIG_ESPHTTPD_SSL_SUPPORT
    if(pFR->httpdFlags & HTTPD_FLAG_SSL)
    {
        bytesWritten = SSL_write(pRconn->ssl, buff, len);
    } else
#endif
    bytesWritten = write(pRconn->fd, buff, len);

    return bytesWritten;
}

void ICACHE_FLASH_ATTR httpdPlatDisconnect(HttpdConnData *pConn) {
    RtosConnType *pRconn = frconn_of_conn(pConn);
    pRconn->needsClose=1;
    pRconn->needWriteDoneNotif=1; //because the real close is done in the writable select code
}

void httpdPlatDisableTimeout(HttpdConnData *pConn) {
    //Unimplemented for FreeRTOS
}

#ifdef linux
//Set/clear global httpd lock.
void ICACHE_FLASH_ATTR httpdPlatLock(HttpdInstance *pInstance) {
    HttpdFreertosInstance *pFR = fr_of_instance(pInstance);
    pthread_mutex_lock(&pFR->httpdMux);
}

void ICACHE_FLASH_ATTR httpdPlatUnlock(HttpdInstance *pInstance) {
    HttpdFreertosInstance *pFR = fr_of_instance(pInstance);
    pthread_mutex_unlock(&pFR->httpdMux);
}
#else
//Set/clear global httpd lock.
void ICACHE_FLASH_ATTR httpdPlatLock(HttpdInstance *pInstance) {
    HttpdFreertosInstance *pFR = fr_of_instance(pInstance);
    xSemaphoreTakeRecursive(pFR->httpdMux, portMAX_DELAY);
}

void ICACHE_FLASH_ATTR httpdPlatUnlock(HttpdInstance *pInstance) {
    HttpdFreertosInstance *pFR = fr_of_instance(pInstance);
    xSemaphoreGiveRecursive(pFR->httpdMux);
}
#endif

void closeConnection(HttpdFreertosInstance *pInstance, RtosConnType *rconn)
{
    httpdDisconCb(&pInstance->httpdInstance, &rconn->connData);

#ifdef CONFIG_ESPHTTPD_SSL_SUPPORT
    if(pInstance->httpdFlags & HTTPD_FLAG_SSL)
    {
        int retval;
        retval = SSL_shutdown(rconn->ssl);
        if(retval == 1)
        {
            ESP_LOGD(TAG, "%s success", "SSL_shutdown()");
        } else if(retval == 0)
        {
            ESP_LOGD(TAG, "%s call again", "SSL_shutdown()");
        } else
        {
            ESP_LOGE(TAG, "%s %d", "SSL_shutdown()", retval);
        }
        ESP_LOGD(TAG, "%s complete", "SSL_shutdown()");
    }
#endif

    close(rconn->fd);
    rconn->fd=-1;

#ifdef CONFIG_ESPHTTPD_SSL_SUPPORT
    if(pInstance->httpdFlags & HTTPD_FLAG_SSL)
    {
        SSL_free(rconn->ssl);
        ESP_LOGD(TAG, "SSL_free() complete");
        rconn->ssl = 0;
    }
#endif
}

#ifdef CONFIG_ESPHTTPD_SSL_SUPPORT
static SSL_CTX* sslCreateContext()
{
    SSL_CTX *ctx = NULL;

    ESP_LOGI(TAG, "SSL server context create ......");

    ctx = SSL_CTX_new(TLS_server_method());
    if (!ctx) {
        ESP_LOGE(TAG, "SSL_CXT_new");
    } else
    {
        ESP_LOGI(TAG, "OK");
    }

    return ctx;
}

/**
 * @return true if successful, false otherwise
 */
static bool sslSetDerCertificateAndKey(HttpdFreertosInstance *pInstance,
                                        const void *certificate, size_t certificate_size,
                                        const void *private_key, size_t private_key_size)
{
    bool status = true;

    ESP_LOGI(TAG, "SSL server context setting ca certificate......");
    int ret = SSL_CTX_use_certificate_ASN1(pInstance->ctx, certificate_size, certificate);
    if (!ret) {
#ifdef linux
        ERR_print_errors_fp(stderr);
#endif
        ESP_LOGE(TAG, "SSL_CTX_use_certificate_ASN1 %d", ret);
        status = false;
    }
    ESP_LOGI(TAG, "OK");

    ESP_LOGI(TAG, "SSL server context setting private key......");
    ret = SSL_CTX_use_RSAPrivateKey_ASN1(pInstance->ctx, private_key, private_key_size);
    if (!ret) {
#ifdef linux
        ERR_print_errors_fp(stderr);
#endif
        ESP_LOGE(TAG, "SSL_CTX_use_RSAPrivateKey_ASN1 %d", ret);
        status = false;
    }

    return status;
}

#endif

#ifdef linux

#define PLAT_TASK_EXIT return NULL

#else

#define PLAT_TASK_EXIT vTaskDelete(NULL)

#endif


PLAT_RETURN platHttpServerTask(void *pvParameters)
{
    int32 listenfd;
    int32 remotefd;
    int32 len;
    int32 ret;
    int x;
    int maxfdp = 0;
    fd_set readset,writeset;
    struct sockaddr name;
    struct sockaddr_in server_addr;
    struct sockaddr_in remote_addr;

    HttpdFreertosInstance *pInstance = (HttpdFreertosInstance*)pvParameters;

    int maxConnections = pInstance->httpdInstance.maxConnections;

#ifdef linux
    pthread_mutex_init(&pInstance->httpdMux, NULL);
#else
    pInstance->httpdMux = xSemaphoreCreateRecursiveMutex();
#endif

    for (x=0; x < maxConnections; x++) {
        pInstance->rconn[x].fd=-1;
    }

#ifdef CONFIG_ESPHTTPD_SHUTDOWN_SUPPORT
    static int currentUdpShutdownPort = 8000;

    struct sockaddr_in udp_addr;
    memset(&udp_addr, 0, sizeof(udp_addr)); /* Zero out structure */
    udp_addr.sin_family = AF_INET;			/* Internet address family */
    udp_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    #ifndef linux
    udp_addr.sin_len = sizeof(udp_addr);
    #endif

    // FIXME: use and increment of currentUdpShutdownPort is not thread-safe
    // and should use a mutex
    pInstance->udpShutdownPort = currentUdpShutdownPort;
    currentUdpShutdownPort++;
    udp_addr.sin_port = htons(pInstance->udpShutdownPort);

    int udpListenfd = socket(AF_INET, SOCK_DGRAM, 0);
    ESP_LOGI(TAG, "udpListenfd %d", udpListenfd);
    ret = bind(udpListenfd, (struct sockaddr *)&udp_addr, sizeof(udp_addr));
    if(ret != 0)
    {
        ESP_LOGE(TAG, "udp bind failure");
        PLAT_TASK_EXIT;
    }
    ESP_LOGI(TAG, "shutdown bound to udp port %d", pInstance->udpShutdownPort);
#endif

    /* Construct local address structure */
    memset(&server_addr, 0, sizeof(server_addr)); /* Zero out structure */
    server_addr.sin_family = AF_INET;			/* Internet address family */
    server_addr.sin_addr.s_addr = pInstance->httpListenAddress.sin_addr.s_addr;
#ifndef linux
    server_addr.sin_len = sizeof(server_addr);
#endif
    server_addr.sin_port = htons(pInstance->httpPort); /* Local port */

    char serverStr[20];
    inet_ntop(AF_INET, &(server_addr.sin_addr), serverStr, sizeof(serverStr));

    /* Create socket for incoming connections */
    do{
        listenfd = socket(AF_INET, SOCK_STREAM, 0);
        if (listenfd == -1) {
            ESP_LOGE(TAG, "socket");
            vTaskDelay(1000/portTICK_PERIOD_MS);
        }
    } while(listenfd == -1);

#ifdef CONFIG_ESPHTTPD_SO_REUSEADDR
    // enable SO_REUSEADDR so servers restarted on the same ip addresses
    // do not require waiting for 2 minutes while the socket is in TIME_WAIT
    int enable = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    {
        perror("setsockopt(SO_REUSEADDR) failed");
    }
#endif

    /* Bind to the local port */
    do{
        ret = bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
        if (ret != 0) {
            ESP_LOGE(TAG, "bind to address %s:%d", serverStr, pInstance->httpPort);
            perror("bind");
            vTaskDelay(1000/portTICK_PERIOD_MS);
        }
    } while(ret != 0);

    do{
        /* Listen to the local connection */
        ret = listen(listenfd, maxConnections);
        if (ret != 0) {
            ESP_LOGE(TAG, "listen on fd %d", listenfd);
            perror("listen");
            vTaskDelay(1000/portTICK_PERIOD_MS);
        }
    } while(ret != 0);

    ESP_LOGI(TAG, "esphttpd: active and listening to connections on %s", serverStr);
    bool shutdown = false;
    bool listeningForNewConnections = false;
    while(!shutdown)
    {
        // clear fdset, and set the select function wait time
        int socketsFull=1;
        maxfdp = 0;
        FD_ZERO(&readset);
        FD_ZERO(&writeset);

        for(x=0; x < maxConnections; x++){
            RtosConnType *pRconn = &(pInstance->rconn[x]);
            if (pRconn->fd!=-1) {
                FD_SET(pRconn->fd, &readset);
                if (pRconn->needWriteDoneNotif) FD_SET(pRconn->fd, &writeset);
                if (pRconn->fd>maxfdp) maxfdp = pRconn->fd;
            } else {
                socketsFull=0;
            }
        }

        if (!socketsFull) {
            FD_SET(listenfd, &readset);
            if (listenfd>maxfdp) maxfdp=listenfd;
            ESP_LOGD(TAG, "Sel add listen %d", listenfd);
            if(!listeningForNewConnections)
            {
                listeningForNewConnections = true;
                ESP_LOGI(TAG, "listening for new connections on '%s'", serverStr);
            }
        } else
        {
            if(listeningForNewConnections)
            {
                listeningForNewConnections = false;
                ESP_LOGI(TAG, "all %d connections in use on '%s'", maxConnections, serverStr);
            }
        }

#ifdef CONFIG_ESPHTTPD_SHUTDOWN_SUPPORT
        FD_SET(udpListenfd, &readset);
        if(udpListenfd > maxfdp) maxfdp = udpListenfd;
#endif

        //polling all exist client handle,wait until readable/writable
        ret = select(maxfdp+1, &readset, &writeset, NULL, NULL);//&timeout
        ESP_LOGD(TAG, "select ret");
        if(ret > 0){
#ifdef CONFIG_ESPHTTPD_SHUTDOWN_SUPPORT
            if (FD_ISSET(udpListenfd, &readset)) {
                shutdown = true;
                ESP_LOGI(TAG, "shutting down");
            }
#endif

            //See if we need to accept a new connection
            if (FD_ISSET(listenfd, &readset)) {
                len=sizeof(struct sockaddr_in);
                remotefd = accept(listenfd, (struct sockaddr *)&remote_addr, (socklen_t *)&len);
                if (remotefd<0) {
                    ESP_LOGE(TAG, "accept failed");
                    perror("accept");
                    continue;
                }
                for(x=0; x < maxConnections; x++) if (pInstance->rconn[x].fd==-1) break;
                if (x == maxConnections) {
                    ESP_LOGE(TAG, "all connections in use, closing fd");
                    close(remotefd);
                    continue;
                }

                RtosConnType *pRconn = &(pInstance->rconn[x]);

                int keepAlive = 1; //enable keepalive
                int keepIdle = 60; //60s
                int keepInterval = 5; //5s
                int keepCount = 3; //retry times

                setsockopt(remotefd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepAlive, sizeof(keepAlive));
                setsockopt(remotefd, IPPROTO_TCP, TCP_KEEPIDLE, (void*)&keepIdle, sizeof(keepIdle));
                setsockopt(remotefd, IPPROTO_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval));
                setsockopt(remotefd, IPPROTO_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount));

                pRconn->fd=remotefd;
                pRconn->needWriteDoneNotif=0;
                pRconn->needsClose=0;

#ifdef CONFIG_ESPHTTPD_SSL_SUPPORT
                if(pInstance->httpdFlags & HTTPD_FLAG_SSL)
                {
                    ESP_LOGD(TAG, "SSL server create .....");
                    pRconn->ssl = SSL_new(pInstance->ctx);
                    if (!pRconn->ssl) {
                        ESP_LOGE(TAG, "SSL_new");
                        close(remotefd);
                        pRconn->fd = -1;
                        continue;
                    }
                    ESP_LOGD(TAG, "OK");

                    SSL_set_fd(pRconn->ssl, pRconn->fd);

                    ESP_LOGD(TAG, "SSL server accept client .....");
                    ret = SSL_accept(pRconn->ssl);
                    if (!ret) {
                        int ssl_error = SSL_get_error(pRconn->ssl, ret);
                        ESP_LOGE(TAG, "SSL_accept %d", ssl_error);
                        close(remotefd);
                        SSL_free(pRconn->ssl);
                        pRconn->fd = -1;
                        continue;
                    }
                    ESP_LOGD(TAG, "OK");
                }
#endif

                len=sizeof(name);
                getpeername(remotefd, &name, (socklen_t *)&len);
                struct sockaddr_in *piname=(struct sockaddr_in *)&name;

                pRconn->port = piname->sin_port;
                memcpy(&pRconn->ip, &piname->sin_addr.s_addr, sizeof(pRconn->ip));

                // NOTE: httpdConnectCb cannot fail
                httpdConnectCb(&pInstance->httpdInstance, &pRconn->connData);
            }

            //See if anything happened on the existing connections.
            for(x=0; x < maxConnections; x++) {
                RtosConnType *pRconn = &(pInstance->rconn[x]);

                //Skip empty slots
                if (pRconn->fd==-1) continue;

                //Check for write availability first: the read routines may write needWriteDoneNotif while
                //the select didn't check for that.
                if (pRconn->needWriteDoneNotif && FD_ISSET(pRconn->fd, &writeset)) {
                    pRconn->needWriteDoneNotif=0; //Do this first, httpdSentCb may write something making this 1 again.
                    if (pRconn->needsClose) {
                        //Do callback and close fd.
                        closeConnection(pInstance, pRconn);
                    } else {
                        if(httpdSentCb(&pInstance->httpdInstance, &pRconn->connData) != CallbackSuccess)
                        {
                            closeConnection(pInstance, pRconn);
                        }
                    }
                }

                if (FD_ISSET(pRconn->fd, &readset)) {
#ifdef CONFIG_ESPHTTPD_SSL_SUPPORT
                    if(pInstance->httpdFlags & HTTPD_FLAG_SSL)
                    {
                        int bytesStillAvailable;

                        // NOTE: we repeat the call to SSL_read() and process data
                        // while SSL indicates there is still pending data.
                        //
                        // select() isn't detecting available data, this
                        // re-read approach resolves an issue where data is stuck in
                        // SSL internal buffers
                        do {
                            ret = SSL_read(pRconn->ssl, &pInstance->precvbuf, RECV_BUF_SIZE - 1);

                            bytesStillAvailable = SSL_has_pending(pRconn->ssl);

                            if(ret <= 0)
                            {
                                int ssl_error = SSL_get_error(pRconn->ssl, ret);
                                if(ssl_error != SSL_ERROR_NONE)
                                {
                                    ESP_LOGE(TAG, "ssl_error %d, ret %d, bytesStillAvailable %d", ssl_error, ret, bytesStillAvailable);
                                } else
                                {
                                    ESP_LOGD(TAG, "ssl_error %d, ret %d, bytesStillAvailable %d", ssl_error, ret, bytesStillAvailable);
                                }
                            }

                            if (ret > 0) {
                                //Data received. Pass to httpd.
                                if(httpdRecvCb(&pInstance->httpdInstance, &pRconn->connData, &pInstance->precvbuf[0], ret) != CallbackSuccess)
                                {
                                    closeConnection(pInstance, pRconn);
                                }
                            } else {
                                //recv error,connection close
                                closeConnection(pInstance, pRconn);
                            }
                        } while(bytesStillAvailable);
                    } else
                    {
#endif
                        ret = recv(pRconn->fd, &pInstance->precvbuf[0], RECV_BUF_SIZE, 0);

                        if (ret > 0) {
                            //Data received. Pass to httpd.
                            if(httpdRecvCb(&pInstance->httpdInstance, &pRconn->connData, &pInstance->precvbuf[0], ret) != CallbackSuccess)
                            {
                                closeConnection(pInstance, pRconn);
                            }
                        } else {
                            //recv error,connection close
                            closeConnection(pInstance, pRconn);
                        }
#ifdef CONFIG_ESPHTTPD_SSL_SUPPORT
                    }
#endif
                }
            }
        }
    }

#ifdef CONFIG_ESPHTTPD_SHUTDOWN_SUPPORT
    close(listenfd);
    close(udpListenfd);

    // close all open connections
    for(x=0; x < maxConnections; x++)
    {
        RtosConnType *pRconn = &(pInstance->rconn[x]);

        if(pRconn->fd != -1)
        {
            closeConnection(pInstance, pRconn);
        }
    }

    ESP_LOGI(TAG, "httpd on %s exiting", serverStr);
    pInstance->isShutdown = true;
#endif /* #ifdef CONFIG_ESPHTTPD_SHUTDOWN_SUPPORT */

    PLAT_TASK_EXIT;
}

#ifdef linux

#include <signal.h>
#include <time.h>

void platform_timer_handler (union sigval val)
{
    HttpdPlatTimerHandle handle = val.sival_ptr;

    // call the callback
    handle->callback(handle->callbackArg);

    // stop the timer if we aren't autoreloading
    if(!handle->autoReload)
    {
        httpdPlatTimerStop(handle);
    }
}

HttpdPlatTimerHandle httpdPlatTimerCreate(const char *name, int periodMs, int autoreload, void (*callback)(void *arg), void *ctx)
{
    struct sigevent event;
    HttpdPlatTimerHandle handle = (HttpdPlatTimerHandle)malloc(sizeof(HttpdPlatTimer));

    handle->autoReload = autoreload;
    handle->callback = callback;
    handle->timerPeriodMS = periodMs;
    handle->callbackArg = ctx;

    event.sigev_notify = SIGEV_THREAD;
    event.sigev_notify_function = platform_timer_handler;
    event.sigev_value.sival_ptr = handle;

    int retval = timer_create(CLOCK_MONOTONIC, &event, &(handle->timer));
    if(retval != 0)
    {
        ESP_LOGE(TAG, "timer_create() %d", retval);
    }

    return handle;
}

void httpdPlatTimerStart(HttpdPlatTimerHandle handle)
{
    struct itimerspec new_value;
    struct itimerspec old_value;
    int seconds = handle->timerPeriodMS / 1000;
    int nsec = ((handle->timerPeriodMS % 1000) * 1000000);
    new_value.it_value.tv_sec = seconds;
    new_value.it_value.tv_nsec = nsec;
    new_value.it_interval.tv_sec = seconds;
    new_value.it_interval.tv_nsec = nsec;
    int retval = timer_settime(handle->timer, 0,
        &new_value,
        &old_value);

    if(retval != 0)
    {
        ESP_LOGE(TAG, "timer_settime() %d", retval);
    }
}

void httpdPlatTimerStop(HttpdPlatTimerHandle handle)
{
    struct itimerspec new_value;
    struct itimerspec old_value;
    memset(&new_value, 0, sizeof(struct itimerspec));
    int retval = timer_settime(handle->timer, 0,
        &new_value,
        &old_value);

    if(retval != 0)
    {
        ESP_LOGE(TAG, "timer_settime() %d", retval);
    }
}

void httpdPlatTimerDelete(HttpdPlatTimerHandle handle)
{
    timer_delete(handle->timer);
    free(handle);
}
#else
HttpdPlatTimerHandle httpdPlatTimerCreate(const char *name, int periodMs, int autoreload, void (*callback)(void *arg), void *ctx)
{
    HttpdPlatTimerHandle ret;
#ifdef ESP32
    ret=xTimerCreate(name, pdMS_TO_TICKS(periodMs), autoreload?pdTRUE:pdFALSE, ctx, callback);
#else
    ret=xTimerCreate((const signed char * const)name, (periodMs / portTICK_PERIOD_MS), autoreload?pdTRUE:pdFALSE, ctx, callback);
#endif
    return ret;
}

void httpdPlatTimerStart(HttpdPlatTimerHandle timer) {
    xTimerStart(timer, 0);
}

void httpdPlatTimerStop(HttpdPlatTimerHandle timer) {
    xTimerStop(timer, 0);
}

void httpdPlatTimerDelete(HttpdPlatTimerHandle timer) {
    xTimerDelete(timer, 0);
}
#endif

//Httpd initialization routine. Call this to kick off webserver functionality.
HttpdInitStatus ICACHE_FLASH_ATTR httpdFreertosInitEx(HttpdFreertosInstance *pInstance,
    const HttpdBuiltInUrl *fixedUrls, int port,
    uint32_t listenAddress,
    void* connectionBuffer, int maxConnections,
    HttpdFlags flags)
{
    HttpdInitStatus status;
    char serverStr[20];
    inet_ntop(AF_INET, &(listenAddress), serverStr, sizeof(serverStr));

    pInstance->httpdInstance.builtInUrls=fixedUrls;
    pInstance->httpdInstance.maxConnections = maxConnections;

    status = InitializationSuccess;
    pInstance->httpPort = port;
    pInstance->httpListenAddress.sin_addr.s_addr = listenAddress;
    pInstance->httpdFlags = flags;
    pInstance->isShutdown = false;

    pInstance->rconn = connectionBuffer;

    ESP_LOGI(TAG, "address %s, port %d, maxConnections %d, mode %s",
            serverStr,
            port, maxConnections, (flags & HTTPD_FLAG_SSL) ? "ssl" : "non-ssl");

    return status;
}

HttpdInitStatus ICACHE_FLASH_ATTR httpdFreertosInit(HttpdFreertosInstance *pInstance,
    const HttpdBuiltInUrl *fixedUrls, int port,
    void* connectionBuffer, int maxConnections,
    HttpdFlags flags)
{
    HttpdInitStatus status;

    status = httpdFreertosInitEx(pInstance, fixedUrls, port, INADDR_ANY,
                    connectionBuffer, maxConnections,
                    flags);
    ESP_LOGI(TAG, "init");

    return status;
}

SslInitStatus ICACHE_FLASH_ATTR httpdFreertosSslInit(HttpdFreertosInstance *pInstance)
{
    SslInitStatus status = SslInitSuccess;

#ifdef CONFIG_ESPHTTPD_SSL_SUPPORT
    if(pInstance->httpdFlags & HTTPD_FLAG_SSL)
    {
        pInstance->ctx = sslCreateContext();
        if(!pInstance->ctx)
        {
            ESP_LOGE(TAG, "create ssl context");
            status = StartFailedSslNotConfigured;
        }
    }
#endif

    return status;
}

void ICACHE_FLASH_ATTR httpdFreertosSslSetCertificateAndKey(HttpdFreertosInstance *pInstance,
                                        const void *certificate, size_t certificate_size,
                                        const void *private_key, size_t private_key_size)
{
#ifdef CONFIG_ESPHTTPD_SSL_SUPPORT
    if(pInstance->httpdFlags & HTTPD_FLAG_SSL)
    {
        if(pInstance->ctx)
        {
            if(!sslSetDerCertificateAndKey(pInstance, certificate, certificate_size,
                                private_key, private_key_size))
            {
                ESP_LOGE(TAG, "sslSetDerCertificate");
            }
        } else
        {
            ESP_LOGE(TAG, "Call httpdFreertosSslInit() first");
        }
    } else
    {
        ESP_LOGE(TAG, "Server not initialized for ssl");
    }
#endif
}

void ICACHE_FLASH_ATTR httpdFreertosSslSetClientValidation(HttpdFreertosInstance *pInstance,
                                         SslClientVerifySetting verifySetting)
{
#ifdef CONFIG_ESPHTTPD_SSL_SUPPORT
    int flags;

    if(pInstance->httpdFlags & HTTPD_FLAG_SSL)
    {
        if(pInstance->ctx)
        {
            if(verifySetting == SslClientVerifyRequired)
            {
                flags = SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
            } else
            {
                flags = SSL_VERIFY_NONE;
            }

            // NOTE: esp32's openssl wraper isn't using the function callback parameter, the last
            // parameter passed.
            SSL_CTX_set_verify(pInstance->ctx, flags, 0);
        } else
        {
            ESP_LOGE(TAG, "Call httpdFreertosSslInit() first");
        }
    } else
    {
        ESP_LOGE(TAG, "Server not initialized for ssl");
    }
#endif
}

void ICACHE_FLASH_ATTR httpdFreertosSslAddClientCertificate(HttpdFreertosInstance *pInstance,
                                          const void *certificate, size_t certificate_size)
{
#ifdef CONFIG_ESPHTTPD_SSL_SUPPORT
    X509 *client_cacert = d2i_X509(NULL, certificate, certificate_size);
    int rv = SSL_CTX_add_client_CA(pInstance->ctx, client_cacert);
    if(rv == 0)
    {
        ESP_LOGE(TAG, "SSL_CTX_add_client_CA failed");
    }
#endif
}

HttpdStartStatus ICACHE_FLASH_ATTR httpdFreertosStart(HttpdFreertosInstance *pInstance)
{
#ifdef CONFIG_ESPHTTPD_SSL_SUPPORT
    if((pInstance->httpdFlags & HTTPD_FLAG_SSL) && !pInstance->ctx)
    {
        ESP_LOGE(TAG, "StartFailedSslNotConfigured");
        return StartFailedSslNotConfigured;
    }
#endif

#ifdef linux
    pthread_t thread;
    pthread_create(&thread, NULL, platHttpServerTask, pInstance);
#else
#ifdef ESP32
#ifndef CONFIG_ESPHTTPD_PROC_CORE
#define CONFIG_ESPHTTPD_PROC_CORE   tskNO_AFFINITY
#endif
#ifndef CONFIG_ESPHTTPD_PROC_PRI
#define CONFIG_ESPHTTPD_PROC_PRI    4
#endif
    xTaskCreatePinnedToCore(platHttpServerTask, (const char *)"esphttpd", HTTPD_STACKSIZE, pInstance, CONFIG_ESPHTTPD_PROC_PRI, NULL, CONFIG_ESPHTTPD_PROC_CORE);
#else
    xTaskCreate(platHttpServerTask, (const signed char *)"esphttpd", HTTPD_STACKSIZE, pInstance, 4, NULL);
#endif
#endif

    ESP_LOGI(TAG, "starting server on port port %d, maxConnections %d, mode %s",
            pInstance->httpPort, pInstance->httpdInstance.maxConnections,
            (pInstance->httpdFlags & HTTPD_FLAG_SSL) ? "ssl" : "non-ssl");

    return StartSuccess;
}

#ifdef CONFIG_ESPHTTPD_SHUTDOWN_SUPPORT
void httpdPlatShutdown(HttpdInstance *pInstance)
{
    int err;
    HttpdFreertosInstance *pFR = fr_of_instance(pInstance);

    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if(s <= 0)
    {
        ESP_LOGE(TAG, "socket %d", s);
    }

    struct sockaddr_in udp_addr;
    memset(&udp_addr, 0, sizeof(udp_addr)); /* Zero out structure */
    udp_addr.sin_family = AF_INET;			/* Internet address family */
    udp_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
#ifndef linux
    udp_addr.sin_len = sizeof(udp_addr);
#endif
    udp_addr.sin_port = htons(pFR->udpShutdownPort);

    while(!pFR->isShutdown)
    {
        ESP_LOGI(TAG, "sending shutdown to port %d", pFR->udpShutdownPort);

        err = sendto(s, pFR, sizeof(pFR), 0,
                (struct sockaddr*)&udp_addr, sizeof(udp_addr));
        if(err != sizeof(pFR))
        {
            ESP_LOGE(TAG, "sendto");
            perror("sendto");
        }

        if(!pFR->isShutdown)
        {
            vTaskDelay(200 / portTICK_PERIOD_MS);
        }
    }

#ifdef CONFIG_ESPHTTPD_SSL_SUPPORT
    if(pFR->httpdFlags & HTTPD_FLAG_SSL)
    {
        SSL_CTX_free(pFR->ctx);
    }
#endif

    close(s);
}
#endif

#endif
