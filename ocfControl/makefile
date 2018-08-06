# Makefile

IOTIVITY_PATH = /home/iain/iot/iotivity/out/linux/x86_64/release/deploy

COMPFLAGS =  -Wall -std=c++11  -I$(IOTIVITY_PATH)/include/resource/stack \
-I$(IOTIVITY_PATH)/include/resource \
-I$(IOTIVITY_PATH)/include/c_common 

LINKFLAGS =  -Wall -std=c++11 
LINKEDLIBS= -L$(IOTIVITY_PATH) -Wl,-rpath=$(IOTIVITY_PATH) \
-loc -loc_internal -loctbstack  -lpthread

.PHONY: all clean

all: ocfSwitch

ocfSwitch: ocfSwitch.o ../ocflightclient.o
	c++ $(LINKFLAGS) -o ocfSwitch ocfSwitch.o ../ocflightclient.o $(LINKEDLIBS)

ocfSwitch.o: ocfSwitch.cpp ../ocflightclient.hpp
	c++ $(COMPFLAGS) -c ocfSwitch.cpp


clean:
	rm -f *.o ocfSwitch
