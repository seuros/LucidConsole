#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project directory, and that the project's root directory contains esp-idf.
#

PROJECT_NAME := LucidConsole

BOARD ?= ideaspark_oled_0.96_v2.1
export BOARD

EXTRA_COMPONENT_DIRS += $(CURDIR)/boards/$(BOARD)


include $(IDF_PATH)/make/project.mk