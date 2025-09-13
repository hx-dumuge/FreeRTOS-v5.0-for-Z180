## Makefile for FreeRTOS V5.4.3 Z180 demo
## by dumuge
## 2024-06-20

CC=sdcc
TARGET=app.hex

SOURCE_DIR = ../Source
PORT_DIR = ../Source/portable/SDCC/Z180

INCLUDE_DIRS = $(SOURCE_DIR)/include $(SOURCE_DIR)/portable/SDCC/Z180 ./Common ./Inc
SOURCE_DIRS	= $(SOURCE_DIR) $(SOURCE_DIR)/portable/MemMang $(SOURCE_DIR)/portable/SDCC/Z180 \
			./Common ./Src .


NO_OPT=--nogcse --noinvariant --noinduction --nojtbound --noloopreverse --nolabelopt --nooverlay --peep-asm
DEBUG=--debug

# --std-sdcc99 --opt-code-size
CFLAGS=-mz180 --no-std-crt0 -V -I. -I./Common/include -I$(SOURCE_DIR)/include -I$(PORT_DIR) $(DEBUG)
APPFLAGS=--code-loc 0x500 --data-loc 0xC000 
# --stack-loc 0xFFF0

SRC	= ./main.c \
$(SOURCE_DIR)/tasks.c \
$(SOURCE_DIR)/queue.c \
$(SOURCE_DIR)/list.c \
$(SOURCE_DIR)/portable/MemMang/heap_1.c \
$(PORT_DIR)/port.c

# Define all object files.
OBJ = $(SRC:.c=.rel)

all: crt0 $(TARGET)

crt0: $(PORT_DIR)/crt0.s
	sdasz80 -xlos -g $<

%.rel: %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(APPFLAGS) $(PORT_DIR)/crt0.rel $^ -o $@

clean:
	$(foreach d, $(SOURCE_DIRS), $(foreach ext, asm rel lst lk noi map sym ihx hex rst adb cdb, rm -f $(d)/*.$(ext));)

