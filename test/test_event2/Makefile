BASE_PATH = ../..
INC = -I/usr/local/include -I $(BASE_PATH)
LIB = -L/usr/local/lib

CXXFLAGS = -std=c++11 -g
LINKFLAGS = -lglog -lpthread -Wl,-R /usr/local/lib

all: event

channel.o: $(BASE_PATH)/evk/channel.cc $(BASE_PATH)/evk/channel.h
	g++ $(CXXFLAGS) -c $< $(INC) $(LIB)

event_loop.o: $(BASE_PATH)/evk/event_loop.cc $(BASE_PATH)/evk/event_loop.h
	g++ $(CXXFLAGS) -c $< $(INC) $(LIB)

poller.o: $(BASE_PATH)/evk/poller.cc $(BASE_PATH)/evk/poller.h
	g++ $(CXXFLAGS) -c $< $(INC) $(LIB)

event: test_event2.cc event_loop.o poller.o channel.o
	g++ $(CXXFLAGS) -o $@ $^ $(INC) $(LIB) $(LINKFLAGS)

.PHONY: clean
clean:
	rm -f *.o event

