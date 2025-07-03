# SSD1306 Override Component
# This component patches the upstream SSD1306 library for ESP8266 compatibility

COMPONENT_ADD_INCLUDEDIRS := .
COMPONENT_SRCDIRS := .

# Include the original SSD1306 component
COMPONENT_DEPENDS := ssd1306