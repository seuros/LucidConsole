# Include the board-specific header file
CFLAGS += -I$(CURDIR)/../../boards/$(BOARD)

# Include subdirectories for headers
COMPONENT_ADD_INCLUDEDIRS := . bus hardware display wifi web uart

# All source files including subdirectories
COMPONENT_SRCDIRS := . bus hardware display wifi web uart

# Component dependencies - add SSD1306 and fonts libraries
COMPONENT_DEPENDS := ssd1306 fonts

# Temporarily exclude files until ESP8266 adaptation complete
COMPONENT_OBJEXCLUDE := hardware/uart_bridge.o bus/i2c_bus.o display/oled_hw_init.o display/oled_hw_render.o display/oled_init.o display/oled_render.o main_minimal.o main_minimal_test.o main_minimal_working.o main_monolithic_backup.o main_full.o main_modular_test.o main_luciduart.o

$(info Building LucidConsole with OLED-focused main.c)
