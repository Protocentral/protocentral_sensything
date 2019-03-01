# Automatically generated build file. Do not edit.
COMPONENT_INCLUDES += $(PROJECT_PATH)/components/libesphttpd/core $(PROJECT_PATH)/components/libesphttpd/espfs $(PROJECT_PATH)/components/libesphttpd/util $(PROJECT_PATH)/components/libesphttpd/include $(PROJECT_PATH)/components/libesphttpd/lib/heatshrink
COMPONENT_LDFLAGS += -L$(BUILD_DIR_BASE)/libesphttpd -lwebpages-espfs -llibesphttpd
COMPONENT_LINKER_DEPS += 
COMPONENT_SUBMODULES += 
COMPONENT_LIBRARIES += libesphttpd
component-libesphttpd-build: 
