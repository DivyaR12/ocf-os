# Makefile
.DEFAULT_GOAL := bridge

KERNEL := $(shell uname -s)
LOWER_KERNEL := $(shell echo $(KERNEL) | tr A-Z a-z)
PROC := $(shell lscpu | grep -e Architecture: |  sed s/Architecture:\\s*//)
IOTIVITY_PATH := $(HOME)/iot/iotivity/out/$(LOWER_KERNEL)/$(PROC)/release/deploy
OSIOT_PATH := $(HOME)/atis-os-iot

COMPFLAGS =  -Wall -std=c++11  -I$(IOTIVITY_PATH)/include/resource/stack \
-I$(IOTIVITY_PATH)/include/resource \
-I$(IOTIVITY_PATH)/include/c_common \
-I$(OSIOT_PATH)/asio-1.10.6/include -I$(OSIOT_PATH)

LINKFLAGS =  -Wall -std=c++11 
LINKEDLIBS= -L$(IOTIVITY_PATH) -Wl,-rpath=$(IOTIVITY_PATH) \
-L$(OSIOT_PATH) -Wl,--gc-sections,-rpath=$(OSIOT_PATH) \
-loc -loc_internal -loctbstack -losiot -lssl -lcrypto -lxerces-c -lcurl -lpthread

.PHONY: all clean server ocfcontrol

all: bridge server ocfcontrol

server:
	cd server && make

ocfcontrol:
	cd ocfControl && make

bridge: bridge.o ocflightclient.o
	c++ $(LINKFLAGS) -o bridge bridge.o ocflightclient.o $(LINKEDLIBS)

bridge.o: bridge.cpp ocflightclient.hpp
	c++ $(COMPFLAGS) -c bridge.cpp

ocflightclient.o: ocflightclient.cpp ocflightclient.hpp
	c++ $(COMPFLAGS) -c ocflightclient.cpp

clean:
	rm -f *.o bridge
