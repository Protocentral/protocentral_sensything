# Libesphttpd intro

Libesphttpd is a HTTP server library for the ESP8266/ESP32. It supports integration in projects
running under the non-os and FreeRTOS-based SDK. Its core is clean and small, but it provides an
extensible architecture with plugins to handle a flash-based compressed read-only filesystem
for static files, a tiny template engine, websockets, a captive portal, and more.

# Examples

There are two example projects that integrate this code, both a [non-os](http://git.spritesserver.nl/esphttpd.git/)
as well as a [FreeRTOS-based](https://github.com/chmorgan/esphttpd-freertos) example. They show
how to use libesphttpd to serve files from an ESP8266/ESP32 and illustrate a way to make an user associate
the ESP8266/ESP32 with an access point from a standard webbrowser on a PC or mobile phone.

# Using with esp-idf (esp32)

Place the libesphttpd repository into the components directory of your esp-idf folder. This should
put it at esp-idf/components/libesphttpd If it is in the correct location you should see a 'ESP-HTTPD Config'
entry under 'Component config' when you run 'make menuconfig' on your esp-idf application.

# SSL Support

Libesphttpd supports https under FreeRTOS via openssl/mbedtls. Server and client certificates are supported.

Enable 'ESPHTTPD_SSL_SUPPORT' during project configuration.

See the 'How to use SSL' section below.

# Programming guide

Programming libesphttpd will require some knowledge of HTTP. Knowledge of the exact RFCs isn't needed,
but it helps if you know the difference between a GET and a POST request, how HTTP headers work,
what an mime-type is and so on. Furthermore, libesphttpd is written in the C language and uses the
libraries available on the ESP8266/ESP32 SDK. It is assumed the developer knows C and has some experience 
with the SDK.

## Initializing libesphttpd

Initializing libesphttpd is usually done in the `user_main()` of your project, but it is not mandatory
to place the call here. Initialization is done by the `httpdInit(builtInUrls, port)` call. The port
is the TCP port the webserver will listen on; the builtInUrls is the CGI list. Only call the `httpdInit`
once, calling it multiple times leads to undefined behaviour.

(As an aside: CGI actually is an abbreviation for Common Gateway Interface, which is a specification
to allow external processes to interface with a non-embedded webserver. The CGI functions mentioned here
have nothing to do with the CGI protocol specification; the term 'CGI' is just used as a quick
handle for a function interpreting headers and generating data to send to the web client.)

The CGI list is an array of the HttpdBuiltInUrl type. Here's an example:
```c
const HttpdBuiltInUrl builtInUrls[]={
	{"/", cgiRedirect, "/index.cgi"},
	{"/index.cgi", cgiMyFunction, NULL},
	{"*", cgiEspFsHook, NULL},
	{NULL, NULL, NULL}
};
```
As you can see, the array consists of a number of entries, with the last entry filled with NULLs. When 
the webserver gets a request, it will run down the list and try to match the URL the browser sent to the
pattern specified in the first argument in the list. If a match is detected, the corresponding CGI 
function is called. This function gets the opportunity to handle the request, but it also can pass
on handling it; if this happens, the webserver will keep going down the list to look for a CGI
with a matching pattern willing to handle the request; if there is none on the list, it will
generate a 404 page itself.

The patterns can also have wildcards: a * at the end of the pattern matches any text. For instance, 
the pattern `/wifi/*` will match requests for `/wifi/index.cgi` and `/wifi/picture.jpg`, but not
for example `/settings/wifi/`. The cgiEspFsHook is used like that in the example: it will be called
on any request that is not handled by the cgi functions earlier in the list.

There also is a third entry in the list. This is an optional argument for the CGI function; its
purpose differs per specific function. If this is not needed, it's okay to put NULL there instead. 

### Sidenote: About the cgiEspFsHook call
While `cgiEspFsHook` isn't handled any different than any other cgi function, it may be useful 
to shortly elaborate what its function is. `cgiEspFsHook` is responsible, on most implementations,
for serving up the static files that are included in the project: static HTML pages, images, Javascript
code etc. Esphttpd doesn't have a built-in method to serve static files: the code responsible
for doing it is plugged into it the same way as any cgi function is. This allows the developer to
leave away the ability to serve static files if it isn't needed, or use a different implementation
that serves e.g. files off the FAT-partition of a SD-card.

## Built-in CGI functions
The webserver provides a fair amount of general-use CGI functions. Because of the structure of 
libesphttpd works and some linker magic in the Makefiles of the SDKs, the compiler will only
include them in the output binary if they're actually used.

* __cgiRedirect__ (arg: URL to redirect to)
This is a convenience function to redirect the browser requesting this URL to a different URL. For
example, an entry like {"/google", cgiRedirect, "http://google.com"} would redirect
all browsers requesting /google to the website of the search giant.

* __cgiRedirectToHostname__ (arg: hostname to redirect to)
If the host as requested by the browser isn't the hostname in the argument, the webserver will do a redirect
to the host instead. If the hostname does match, it will pass on the request.

* __cgiRedirectApClientToHostname__ (arg: hostname to redirect to)
This does the same as `cgiRedirectToHostname` but only to clients connected to the SoftAP of the
ESP8266/ESP32. This and the former function are used with the captive portal mode. The captive portal consists
of a DNS-server (started by calling `captdnsInit()`) resolving all hostnames into the IP of the
ESP8266/ESP32. These redirect functions can then be used to further redirect the client to the hostname of 
the ESP8266/ESP32.

* Flash updating functions (OTA) - see [README-flash_api](./README-flash_api.md)

* __cgiWiFi* functions__ (arg: various)
These are used to change WiFi mode, scan for access points, associate to an access point etcetera. See
the example projects for an implementation that uses this function call.  [FreeRTOS Example](https://github.com/chmorgan/esphttpd-freertos)

* __cgiWebsocket__ (arg: connect function)
This CGI is used to set up a websocket. Websockets are described later in this document.  See
the example projects for an implementation that uses this function call.  [FreeRTOS Example](https://github.com/chmorgan/esphttpd-freertos)

* __cgiEspFsHook__ (arg: none)
Serves files from the espfs filesystem. The espFsInit function should be called first, with as argument
a pointer to the start of the espfs binary data in flash. The binary data can be both flashed separately
to a free bit of SPI flash, as well as linked in with the binary. The nonos example project can be
configured to do either.

* __cgiEspFsTemplate__ (arg: template function)
The espfs code comes with a small but efficient template routine, which can fill a template file stored on
the espfs filesystem with user-defined data.

* __cgiEspVfsGet__ (arg: base filesystem path)
This is a catch-all cgi function. It takes the url passed to it, looks up the corresponding path in the filesystem and if it exists, sends the file. This simulates what a normal webserver would do with static files.  If the file is not found, (or if http method is not GET) this cgi function returns NOT_FOUND, and then other cgi functions specified later in the routing table can try.  See the example projects for an implementation that uses this function call.  [FreeRTOS Example](https://github.com/chmorgan/esphttpd-freertos)

  The cgiArg value is the base directory path, if specified.
  Usage:
    * ROUTE_CGI("*", cgiEspVfsGet)
    * ROUTE_CGI_ARG("*", cgiEspVfsGet, "/base/directory/")
    * ROUTE_CGI_ARG("*", cgiEspVfsGet, ".") to use the current working directory
    
* __cgiEspVfsUpload__ (arg: base filesystem path)
This is a POST and PUT handler for uploading files to the VFS filesystem.  See the example projects for an implementation that uses this function call.  [FreeRTOS Example](https://github.com/chmorgan/esphttpd-freertos)

  Specify base directory (with trailing slash) or single file as 1st cgiArg.
  If http method is not PUT or POST, this cgi function returns NOT_FOUND, and then other cgi functions specified later in the routing table can try.
  
  Filename can be specified 3 ways, in order of priority lowest to highest:
  1. ___URL Path___  i.e. PUT http://1.2.3.4/path/newfile.txt
  2. ___Inside multipart/form-data___ (todo not supported yet)
  3. ___URL Parameter___  i.e. POST http://1.2.3.4/upload.cgi?filename=path%2Fnewfile.txt
  
  Usage:
    * ROUTE_CGI_ARG("*", cgiEspVfsUpload, "/base/directory/")
      - Allows creating/replacing files anywhere under "/base/directory/".  Don't forget to specify trailing slash in cgiArg!
      - example: POST or PUT http://1.2.3.4/anydir/anyname.txt
    * ROUTE_CGI_ARG("/filesystem/upload.cgi", cgiEspVfsUpload, "/base/directory/")
      - Allows creating/replacing files anywhere under "/base/directory/".  Don't forget to specify trailing slash in cgiArg!
      - example: POST or PUT http://1.2.3.4/filesystem/upload.cgi?filename=newdir%2Fnewfile.txt
    * ROUTE_CGI_ARG("/writeable_file.txt", cgiEspVfsUpload, "/base/directory/writeable_file.txt")
      - Allows only replacing content of one file at "/base/directory/writeable_file.txt".
      - example: POST or PUT http://1.2.3.4/writeable_file.txt

## How to configure and use SSL

### How to create certificates

	SSL servers require certificates. Steps to use:
	- Place a 'cacert.der' and 'prvtkey.der' files in your app directory.

	- To create self certified certificates:
		$ openssl req -sha256 -newkey rsa:4096 -nodes -keyout key.pem -x509 -days 365 -out certificate.pem

	- To generate .der certificates/keys from .pem certificates/keys:
		$ openssl x509 -outform der -in certificate.pem -out certificate.der
		$ openssl rsa -outform der -in key.pem -out key.der

### Compile certificates into your binary image (option 1) OR
	- Create a 'component.mk' file in your app directory and add these lines to it:
		COMPONENT_EMBED_TXTFILES := cacert.der
		COMPONENT_EMBED_TXTFILES += prvtkey.der

And use the below code to gain access to these embedded files. Note the filename with extension is used to generate the binary variables, you can modify the embedded filenames but make sure to update the _binary_xxxx_yyy_start and end entries:

```c
extern const unsigned char cacert_der_start[] asm("_binary_cacert_der_start");
extern const unsigned char cacert_der_end[]   asm("_binary_cacert_der_end");
const size_t cacert_der_bytes = cacert_der_end - cacert_der_start;

extern const unsigned char prvtkey_der_start[] asm("_binary_prvtkey_der_start");
extern const unsigned char prvtkey_der_end[]   asm("_binary_prvtkey_der_end");
const size_t prvtkey_der_bytes = prvtkey_der_end - prvtkey_der_start;
```

### Store / load certificates to a filesystem (option 2)
See the mkspiffs documentation for more information on creating a spiffs filesystem and loading it at runtime.

Otherwise use standard file functions, fopen/fread/fclose to read the certiricates into memory so they can be passed into libesphttpd.

### Configure libesphttpd for ssl and load the server certificate and private key
```c
    httpdFreertosSslInit(&httpdFreertosInstance); // configure libesphttpd for ssl

    // load the server certificate and private key
    httpdFreertosSslSetCertificateAndKey(&httpdFreertosInstance,
                                        cacert_der_ptr, cacert_der_size,
                                        prvtkey_der_ptr, prvtkey_der_size);
```

### Optionally enable client certificate validation (client certificate validation is disabled by default) and load a series of client certificates

You can embed client certificates into the flash image or store them in a filesystem depending on your need.

```c
    SslVerifySetting verifySetting = SslClientVerifyRequired;
    httpdFreertosSslSetClientValidation(&httpdFreertosInstance,
                                        verifySetting);
    httpdFreertosSslAddClientCertificate(&httpdFreertosInstance,
                                         client_certificate_ptr, client_certificate_size);
```

## Writing a CGI function

A CGI function, in principle, is called when the HTTP headers have come in and the client is waiting for
the response of the webserver. The CGI function is responsible for generating this response, including
the correct headers and an appropriate body. To decide what response to generate and what other actions
to take, the CGI function can inspect various information sources, like data passed as GET- or 
POST-arguments.

A simple CGI function may, for example, greet the user with a name given as a GET argument:

```c
CgiStatus ICACHE_FLASH_ATTR cgiGreetUser(HttpdConnData *connData) {
	int len;			//length of user name
	char name[128];		//Temporary buffer for name
	char output[256];	//Temporary buffer for HTML output
	
	//If the browser unexpectedly closes the connection, the CGI will be called 
	//after the isConnectionClosed flag is set. We can use this to clean up any data. It's not
	//used in this simple CGI function.
	if (connData->isConnectionClosed) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}

	if (connData->requestType!=HTTPD_METHOD_GET) {
		//Sorry, we only accept GET requests.
		httpdStartResponse(connData, 406);  //http error code 'unacceptable'
		httpdEndHeaders(connData);
		return HTTPD_CGI_DONE;
	}

	//Look for the 'name' GET value. If found, urldecode it and return it into the 'name' var.
	len=httpdFindArg(connData->getArgs, "name", name, sizeof(name));
	if (len==-1) {
		//If the result of httpdFindArg is -1, the variable isn't found in the data.
		strcpy(name, "unknown person");
	} else {
		//If len isn't -1, the variable is found and is copied to the 'name' variable
	}
	
	//Generate the header
	//We want the header to start with HTTP code 200, which means the document is found.
	httpdStartResponse(connData, 200); 
	//We are going to send some HTML.
	httpdHeader(connData, "Content-Type", "text/html");
	//No more headers.
	httpdEndHeaders(connData);
	
	//We're going to send the HTML as two pieces: a head and a body. We could've also done
	//it in one go, but this demonstrates multiple ways of calling httpdSend.
	//Send the HTML head. Using -1 as the length will make httpdSend take the length
	//of the zero-terminated string it's passed as the amount of data to send.
	httpdSend(connData, "<html><head><title>Page</title></head>", -1)
	//Generate the HTML body. 
	len=sprintf(output, "<body><p>Hello, %s!</p></body></html>", name);
	//Send HTML body to webbrowser. We use the length as calculated by sprintf here.
	//Using -1 again would also have worked, but this is more efficient.
	httpdSend(connData, output, len);

	//All done.
	return HTTPD_CGI_DONE;
}
```

Putting this CGI function into the HttpdBuiltInUrl array, for example with pattern `"/hello.cgi"`,
would allow an user to request the page `"http://192.168.4.1/hello.cgi?name=John+Doe"` and get a document
saying *"Hello, John Doe!"*.

A word of warning: while it may look like you could forego the entire 
httpdStartResponse/httpdHeader/httpdEndHeader structure and send all the HTTP headers using httpdSend,
this will break a few things that need to know when the headers are finished, for example the
HTTP 1.1 chunked transfer mode.

The approach of parsing the arguments, building up a response and then sending it in one go is pretty
simple and works just fine for small bits of data. The gotcha here is that all http data sent during the 
CGI function (headers and data) are temporarily stored in a buffer, which is sent to the client when
the function returns. The size of this buffer is typically about 2K; if the CGI tries to send more than
this, data will be lost. 

The way to get around this is to send part of the data using `httpdSend` and then return with `HTTPD_CGI_MORE`
instead of `HTTPD_CGI_DONE`. The webserver will send the partial data and will call the CGI function
again so it can send another part of the data, until the CGI function finally returns with `HTTPD_CGI_DONE`.
The CGI can store it's state in connData->cgiData, which is a freely usable pointer that will persist across
all calls in the request. It is NULL on the first call, and the standard way of doing things is to allocate 
a pointer to a struct that stores state here. Here's an example:

```c
typedef struct {
	char *stringPos;
} LongStringState;

static char *longString="Please assume this is a very long string, way too long to be sent"\
		"in one time because it won't fit in the send buffer in it's entirety; we have to"\
		"break up sending it in multiple parts."

CgiStatus ICACHE_FLASH_ATTR cgiSendLongString(HttpdConnData *connData) {
	LongStringState *state=connData->cgiData;
	int len;
	
	//If the browser unexpectedly closes the connection, the CGI will be called 
	//after isConnectionClosed is set to true. We can use this to clean up any data. It's pretty relevant
	//here because otherwise we may leak memory when the browser aborts the connection.
	if (connData->isConnectionClosed) {
		//Connection aborted. Clean up.
		if (state!=NULL) free(state);
		return HTTPD_CGI_DONE;
	}

	if (state==NULL) {
		//This is the first call to the CGI for this webbrowser request.
		//Allocate a state structure.
		state=malloc(sizeof(LongStringState);
		//Save the ptr in connData so we get it passed the next time as well.
		connData->cgiData=state;
		//Set initial pointer to start of string
		state->stringPos=longString;
		//We need to send the headers before sending any data. Do that now.
		httpdStartResponse(connData, 200); 
		httpdHeader(connData, "Content-Type", "text/plain");
		httpdEndHeaders(connData);
	}

	//Figure out length of string to send. We will never send more than 128 bytes in this example.
	len=strlen(state->stringPos); //Get remaining length
	if (len>128) len=128; //Never send more than 128 bytes
	
	//Send that amount of data
	httpdSend(connData, state->stringPos, len);
	//Adjust stringPos to first byte we haven't sent yet
	state->stringPos+=len;
	//See if we need to send more
	if (strlen(state->stringPos)!=0) {
		//we have more to send; let the webserver call this function again.
		return HTTPD_CGI_MORE;
	} else {
		//We're done. Clean up here as well: if the CGI function returns HTTPD_CGI_DONE, it will
		//not be called again.
		free(state);
		return HTTPD_CGI_DONE;
	}
}

```
In this case, the CGI is called again after each chunk of data has been sent over the socket. If you need to suspend the
HTTP response and resume it asynchronously for some other reason, you may save the `HttpdConnData` pointer, return
`HTTPD_CGI_MORE`, then later call `httpdContinue` with the saved connection pointer. For example, if you need to
communicate with another device over a different connection, you could send data to that device in the initial CGI call,
then return `HTTPD_CGI_MORE`, then, in the `espconn_recv_callback` for the response, you can call `httpdContinue` to
resume the HTTP response with data retrieved from the other device.

For POST data, a similar technique is used. For small amounts of POST data (smaller than MAX_POST, typically
1024 bytes) the entire thing will be stored in `connData->post->buff` and is accessible in its entirely
on the first call to the CGI function. For example, when using POST to send form data, if the amount of expected
data is low, it is acceptable to do a call like `len=httpdFindArg(connData->post->buff, "varname", buff, sizeof(buff));`
to get the data for the individual form elements.

In all cases, `connData->post->len` will contain the length of the entirety of the POST data, while 
`connData->post->buffLen` contains the length of the data in `connData->post->buff`. In the case where the
total POST data is larger than the POST buffer, the latter will be less than the former. In this case, the 
CGI function is expected to not send any headers or data out yet, but to process the incoming bit of POST data and
return with `HTTPD_CGI_MORE`. The next call will contain the next chunk of POST data. `connData->post->received`
will always contain the total amount of POST data received for the request, including the data passed
to the CGI. When that number equals `connData->post->len`, it means no more POST data is expected and 
the CGI function is free to send out the reply headers and data for the request.

## The template engine

The espfs driver comes with a tiny template engine, which allows for runtime-calculated value changes in a static
html page. It can be included in the builtInUrls variable like this:

```c
	{"/showname.tpl", cgiEspFsTemplate, tplShowName}
```

It requires two things. First of all, the template is needed, which specifically is a file on the espfs with the
same name as the first argument of the builtInUrls value, in this case `showname.tpl`. It is a standard HTML file
containing a number of %name% entries. For example:

```html
<html>
<head><title>Welcome</title></head>
<body>
<h1>Welcome, %username%, to the %thing%!</h1>
</body>
</html>
```

When this URL is requested, the words between percent characters will invoke the `tplShowName` function, allowing
it to output specific data. For example:

```c
CgiStatus ICACHE_FLASH_ATTR tplShowName(HttpdConnData *connData, char *token, void **arg) {
	if (token==NULL) return HTTPD_CGI_DONE;

	if (os_strcmp(token, "username")==0) httpdSend(connData, "John Doe", -1);
	if (os_strcmp(token, "thing")==0) httpdSend(connData, "ESP8266/ESP32 webserver", -1);

	return HTTPD_CGI_DONE;
}
```

This will result in a page stating *Welcome, John Doe, to the ESP8266/ESP32 webserver!*.


## Websocket functionality

ToDo: document this

# Linux support

Lwip provides a POSIX interface that matches that of a Linux system. FreeRTOS primitives are also similiar
to those provided under POSIX.

Running	on a Linux system enables testing under a range of different conditions including different
native pointer sizes (64bit vs.	32bit),	as well	as with	different compilers. These differences can
help reveal portability issues.

Linux tools such as valgrind can be used to check for memory leaks that would be much more difficult
to detect on an	embedded platform. Valgrind and	other tools also provide ways of looking at application
performance that go beyond what	is typically available in an embedded environment.

See https://github.com/chmorgan/libesphttpd_linux_example for an example of how to use libesphttpd under
Linux.

# Licensing

libesphttpd is licensed under the MPLv2. It was originally licensed under a 'Beer-ware' license
by Jeroen Domburg that was equivalent to a public domain license. Chris Morgan
<chmorgan@gmail.com> initiated relicensing to MPLv2 prior
to investing a number of hours into improving the library for its use in a potential
commercial project. The relicensing was done after asking for and receiving
the blessing from most of the projects contributors although it should be noted that the
original license didn't require permission to relicense or use in any way.

The topic of licenses can be controversial. The original license was more free in that it allowed users to use the code
in any way, including relicensing it to any license they chose. The MPLv2 restricts freedom in that it requires
modifications to be given back to the community. This license establishes the agreement that in exchange for using
this great library that users are required to give back their changes to let others benefit.
This was the spirit and intention of the relicencing to the MPLv2.

While the 'Beer-ware' license text was removed to avoid license confusion the authors of this great library,
especially Jeroen, deserve a beer. If you appreciate the library and you meet them in person some day please consider
buying them a beer to say thanks!
