# Makefile

KERNEL := $(shell uname -s)
LOWER_KERNEL := $(shell echo $(KERNEL) | tr A-Z a-z)
PROC := $(shell lscpu | grep -e Architecture: |  sed s/Architecture:\\s*//)
IOTIVITY_PATH := $(HOME)/iot/iotivity/out/$(LOWER_KERNEL)/$(PROC)/release/deploy
OSIOT_PATH := $(HOME)/atis-os-iot

COMPFLAGS =  -Wall -std=c++11  -I$(IOTIVITY_PATH)/include/resource/stack \
-I$(IOTIVITY_PATH)/include/resource \
-I$(IOTIVITY_PATH)/include/c_common

LINKFLAGS =  -Wall -std=c++11 
LINKEDLIBS = -L$(IOTIVITY_PATH) -Wl,-rpath=$(IOTIVITY_PATH) \
-loc -loc_internal -loctbstack

ifdef GPIO
LINKEDLIBS += -lsoc -l96BoardsGPIO 
COMPFLAGS += -DGPIO="TRUE" 
endif


all: server

server: server.o
	c++ $(LINKFLAGS) -o server server.o $(LINKEDLIBS)

server.o: server.cpp
	c++ $(COMPFLAGS) -c server.cpp

clean:
	rm -f server server.o

