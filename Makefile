PROJECT_NAME := iot-multisensor

EXTRA_COMPONENT_DIRS := $(CURDIR)/../esp-idf-lib/components/dht $(CURDIR)/../esp-idf-lib/components/esp_idf_lib_helpers
EXCLUDE_COMPONENTS := max7219 mcp23x17 led_strip
include $(IDF_PATH)/make/project.mk
