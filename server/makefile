# Makefile

COMPFLAGS =  -Wall -std=c++11  -I/home/iain/iot/iotivity/out/linux/x86_64/release/deploy/include/resource/stack \
-I/home/iain/iot/iotivity/out/linux/x86_64/release/deploy/include/resource \
-I/home/iain/iot/iotivity/out/linux/x86_64/release/deploy/include/c_common

LINKFLAGS =  -Wall -std=c++11 
LINKEDLIBS= -L/home/iain/iot/iotivity/out/linux/x86_64/release/deploy -loc -loc_internal -loctbstack -Wl,-rpath=/home/iain/iot/iotivity/out/linux/x86_64/release/deploy

all: server

server: server.o
	c++ $(LINKFLAGS) -o server server.o $(LINKEDLIBS)

server.o: server.cpp
	c++ $(COMPFLAGS) -c server.cpp

clean:
	rm -f server server.o

