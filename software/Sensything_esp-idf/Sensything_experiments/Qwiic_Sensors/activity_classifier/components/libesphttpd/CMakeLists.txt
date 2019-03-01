cmake_minimum_required(VERSION 3.2.2)

include(CheckCCompilerFlag)

function(enable_c_compiler_flag_if_supported flag)
    string(FIND "${CMAKE_C_FLAGS}" "${flag}" flag_already_set)
    if(flag_already_set EQUAL -1)
        check_c_compiler_flag("${flag}" flag_supported)
        if(flag_supported)
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flag}" PARENT_SCOPE)
        endif()
    endif()
endfunction()

enable_c_compiler_flag_if_supported("-Wall")
enable_c_compiler_flag_if_supported("-Werror")

# Enable when heatshink fallthrough bug is fixed, see https://github.com/atomicobject/heatshrink/pull/46
#enable_c_compiler_flag_if_supported("-Wextra")


add_library(esphttpd
    core/auth.c
    core/base64.c
    core/httpdespfs.c
    core/httpd.c
    core/httpd-freertos.c
    core/sha1.c
    core/linux/esp_log.c
    espfs/espfs.c
    util/cgiwebsocket.c
    util/cgiredirect.c
)

set(ENABLE_SSL_SUPPORT 1)

if(ENABLE_SSL_SUPPORT)
    target_compile_definitions(esphttpd PUBLIC "CONFIG_ESPHTTPD_SSL_SUPPORT=1")
endif()

target_compile_definitions(esphttpd PUBLIC "CONFIG_LOG_DEFAULT_LEVEL=ESP_LOG_INFO")

target_compile_definitions(esphttpd PUBLIC "CONFIG_ESPHTTPD_SO_REUSEADDR")
target_compile_definitions(esphttpd PUBLIC "CONFIG_ESPHTTPD_SHUTDOWN_SUPPORT")

target_include_directories(esphttpd PUBLIC "core")
target_include_directories(esphttpd PUBLIC "include")
target_include_directories(esphttpd PUBLIC "espfs")
target_include_directories(esphttpd PUBLIC "include/linux")


add_executable(mkespfsimage
    espfs/mkespfsimage/main.c
    espfs/mkespfsimage/heatshrink_encoder.c
)

find_package(ZLIB REQUIRED)
target_link_libraries(mkespfsimage ${ZLIB_LIBRARIES})

if(ENABLE_SSL_SUPPORT)
    find_package(OpenSSL REQUIRED)
    target_link_libraries(esphttpd ${OPENSSL_LIBRARIES})
    include_directories(${OPENSSL_INCLUDE_DIRS})
endif()

target_compile_definitions(mkespfsimage PUBLIC "ESPFS_HEATSHRINK=1")
target_compile_definitions(mkespfsimage PUBLIC "ESPFS_GZIP=1")

target_include_directories(mkespfsimage PUBLIC "include")
target_include_directories(mkespfsimage PUBLIC "espfs")
target_include_directories(mkespfsimage PUBLIC "lib/heatshrink")


install(TARGETS esphttpd DESTINATION lib)
install(TARGETS mkespfsimage DESTINATION bin)
install(FILES include/libesphttpd/httpd.h DESTINATION include/libesphttpd)
install(FILES include/libesphttpd/httpd-freertos.h DESTINATION include/libesphttpd)
install(FILES include/libesphttpd/cgiwebsocket.h DESTINATION include/libesphttpd)
install(FILES include/libesphttpd/cgiredirect.h DESTINATION include/libesphttpd)
install(FILES include/libesphttpd/httpdespfs.h DESTINATION include/libesphttpd)
install(FILES include/libesphttpd/linux.h DESTINATION include/libesphttpd)
install(FILES include/libesphttpd/platform.h DESTINATION include/libesphttpd)
install(FILES include/libesphttpd/route.h DESTINATION include/libesphttpd)
install(FILES include/libesphttpd/espfs.h DESTINATION include/libesphttpd)
install(FILES include/libesphttpd/webpages-espfs.h DESTINATION include/libesphttpd)
install(FILES include/libesphttpd/esp.h DESTINATION include/libesphttpd)
