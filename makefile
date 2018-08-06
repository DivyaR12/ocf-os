# Makefile

IOTIVITY_PATH = /home/iain/iot/iotivity/out/linux/x86_64/release/deploy
OSIOT_PATH = /home/iain/atis-os-iot

COMPFLAGS =  -Wall -std=c++11  -I$(IOTIVITY_PATH)/include/resource/stack \
-I$(IOTIVITY_PATH)/include/resource \
-I$(IOTIVITY_PATH)/include/c_common \
-I$(OSIOT_PATH)/asio-1.10.6/include -I$(OSIOT_PATH)

LINKFLAGS =  -Wall -std=c++11 
LINKEDLIBS= -L$(IOTIVITY_PATH) -Wl,-rpath=$(IOTIVITY_PATH) \
-L$(OSIOT_PATH) -Wl,--gc-sections,-rpath=$(OSIOT_PATH) \
-loc -loc_internal -loctbstack -losiot -lssl -lcrypto -lxerces-c -lcurl -lpthread

.PHONY: all clean

all: client

client: client.o ocflightclient.o
	c++ $(LINKFLAGS) -o client client.o ocflightclient.o $(LINKEDLIBS)

client.o: client.cpp ocflightclient.hpp
	c++ $(COMPFLAGS) -c client.cpp

ocflightclient.o: ocflightclient.cpp ocflightclient.hpp
	c++ $(COMPFLAGS) -c ocflightclient.cpp

clean:
	rm -f *.o client
