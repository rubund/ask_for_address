MAKEFILE_NAME := $(MAKEFILE_LIST)

SDK_PATH := ../../sdk17
TEMPLATE_PATH = $(abspath $(SDK_PATH)/modules/nrfx/mdk)

#SDK_PATH := ../../sdk11/
#TEMPLATE_PATH = ../../mdk/
LINKER_SCRIPT=nrf52833_xxaa.ld
DEVFAMILY=NRF52
DUT ?= 0
LED0_GPIO := 13
UART_TX_PIN := 6
UART_RTS_PIN := 5
UART_CTS_PIN := 7

DEVKITID ?= 683957460

INC_PATHS  ?= 
INC_PATHS  += -I$(abspath $(SDK_PATH)/modules/nrfx/templates/nRF52832)

include ../common.mk

CFLAGS ?=
CFLAGS += -DNRF52833_XXAA
CFLAGS += -DHAS_UARTE
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16

ASMFLAGS ?=
ASMFLAGS += -DNRF52833_XXAA

LDFLAGS ?=
LDFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16

C_SOURCE_FILES += \
../../src/shared.c \
$(MAINPROG) \
$(abspath $(TEMPLATE_PATH)/system_nrf52833.c) \

ASM_SOURCE_FILES  = $(abspath $(TEMPLATE_PATH)/gcc_startup_nrf52833.S)

include ../common-m4.mk


include ../common-bottom.mk


