BASE_PATH = ~/Coding/evk
INC = -I/usr/local/include -I ~/Coding
LIB = -L/usr/local/lib

CXXFLAGS = -std=c++11 -g
LINKFLAGS = -lglog -lpthread -Wl,-R /usr/local/lib

all: event_loop

event_loop: test_event_loop.cc $(BASE_PATH)/event_loop.cc
	g++ $(CXXFLAGS) -o $@ $^ $(INC) $(LIB) $(LINKFLAGS)

.PHONY: clean
clean:
	rm -f *.o event_loop

