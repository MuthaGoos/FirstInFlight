#This is the top level makefile that will build the entire project space

#Name
NAME = flight


#compiler
CC = /usr/bin/gcc -c
LD = /usr/bin/gcc -o

#FLAGS
FLAGS =-Wall


#DIRECTORIES
ROOT = /home/flight/Desktop/repositories/FirstInFlight
COMMS_DIR = $(ROOT)/comms
COMMON_DIR = $(ROOT)/common
CONTROLLER_DIR = $(ROOT)/controller
GYRO_DIR = $(ROOT)/gyro
H_DIR = $(ROOT)/includes
MOTORS_DIR = $(ROOT)/motors
OUT_DIR = $(ROOT)

#HEADERS
HEADERS = 
HEADERS += $(H_DIR)/debug.h

#LIBS
LIBS =-lpthread


export NAME

#compiler
export CC LD

#headers
export HEADERS

#directories
export H_DIR COMMON_DIR COMMS_DIR CONTROLLER_DIR GYRO_DIR MOTORS_DIR OUT_DIR

#flags
export FLAGS

default:
	@echo "Building Communications"
	@make --no-print-directory -f $(COMMS_DIR)/Makefile.comms
	@echo "Building Common"
	@make --no-print-directory -f $(COMMON_DIR)/Makefile.common
	@echo "Building Controller"
	@make --no-print-directory -f $(CONTROLLER_DIR)/Makefile.controller
	@echo "Building Gyro"
	@make --no-print-directory -f $(GYRO_DIR)/Makefile.gyro
	@echo "Building Motors"
	@make --no-print-directory -f $(MOTORS_DIR)/Makefile.motors


all: default


clean:
	@echo "Cleaning Communications"
	@make --no-print-directory -f $(COMMS_DIR)/Makefile.comms clean
	@echo "Cleaning Common"
	@make --no-print-directory -f $(COMMON_DIR)/Makefile.common clean
	@echo "Cleaning Controller"
	@make --no-print-directory -f $(CONTROLLER_DIR)/Makefile.controller clean
	@echo "Cleaning Gyro"
	@make --no-print-directory -f $(GYRO_DIR)/Makefile.gyro clean
	@echo "Cleaning Motors"
	@make --no-print-directory -f $(MOTORS_DIR)/Makefile.motors clean
	rm $(OUT_DIR)/$(NAME)


