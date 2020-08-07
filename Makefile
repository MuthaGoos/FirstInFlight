#This is the top level makefile that will build the entire project space

#Name
EXE = flight

ROOT = $(shell pwd)

#FLAGS
CFLAGS =-Wall

#DIRECTORIES
HEADER_DIR = "$(ROOT)/includes"
BUILD_DIR = "$(ROOT)/build"

#compiler
CC = /usr/bin/gcc -c -I $(HEADER_DIR)
LD = /usr/bin/gcc -o 

#LIBS
LIBS =-lpthread


export  CC         \
	LD         \
	CFLAGS     \
	BUILD_DIR 

default:
	-mkdir -p $(BUILD_DIR)
	@echo "Building Common"
	@make -C common
	@echo "Building Hardware Interfaces"
	@make -C hardware
	@echo "Building Controller"
	@make -C controller
	@echo "Building Communications"
	@make -C comms
	@echo "Building Simulation"
	@make -C simulation
	gcc -o flight build/*.o

all: default 

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(EXE)
