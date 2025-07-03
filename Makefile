#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project directory, and that the project's root directory contains esp-idf.
#

PROJECT_NAME := LucidConsole

# Terminal environment check (If you're going to Lucid, you better be in the dark)
SHELL := /bin/bash
UNAME := $(shell uname -s)

# Verify we're in a proper terminal environment
ifeq ($(OS),Windows_NT)
    $(error Windows detected. Use WSL2 or switch to Linux/macOS. LucidConsole requires a real terminal.)
endif

BOARD ?= ideaspark_oled_0.96_v2.1
export BOARD

EXTRA_COMPONENT_DIRS += $(CURDIR)/boards/$(BOARD)

# Linux-style patching system for submodules
PATCHES_DIR := $(CURDIR)/components/patches
SSD1306_DIR := $(CURDIR)/components/ssd1306

# Check if patch is needed and apply it
.PHONY: patch-components
patch-components:
	@if [ -d "$(SSD1306_DIR)" ]; then \
		if ! grep -q "0x7f" "$(SSD1306_DIR)/ssd1306/ssd1306.c" 2>/dev/null; then \
			echo "Applying ESP8266 compatibility patches..."; \
			cd "$(SSD1306_DIR)" && \
			patch -p1 -N < "$(PATCHES_DIR)/0001-ssd1306-reduce-power-prevent-brownout.patch" || true; \
			echo "✓ Patches applied"; \
		fi \
	fi

# Hook into the build process
app-flash: patch-components
app: patch-components

include $(IDF_PATH)/make/project.mk