BASE_PATH = ../..
SRC = $(BASE_PATH)/evk
INC = -I/usr/local/include -I $(BASE_PATH)
LIB = -L/usr/local/lib

CXXFLAGS = -std=c++11 -g
LINKFLAGS = -lglog -lpthread -Wl,-R /usr/local/lib

all: timer 

channel.o: $(SRC)/channel.cc $(SRC)/channel.h
	g++ $(CXXFLAGS) -c $< 

invoke_timer.o: $(SRC)/invoke_timer.cc $(SRC)/invoke_timer.h
	g++ $(CXXFLAGS) -c $<

timestamp.o: $(SRC)/timestamp.cc $(SRC)/timestamp.h
	g++ $(CXXFLAGS) -c $<

timer_queue.o: $(SRC)/timer_queue.cc $(SRC)/timer_queue.h
	g++ $(CXXFLAGS) -c $<

event_loop.o: $(SRC)/event_loop.cc $(SRC)/event_loop.h
	g++ $(CXXFLAGS) -c $<

poller.o: $(SRC)/poller.cc $(SRC)/poller.h
	g++ $(CXXFLAGS) -c $<

socket_ops.o: $(SRC)/socket_ops.cc $(SRC)/socket_ops.h
	g++ $(CXXFLAGS) -c $<

timer: test_invoke_timer.cc event_loop.o poller.o timer_queue.o \
	channel.o invoke_timer.o timestamp.o socket_ops.o
	g++ $(CXXFLAGS) -o $@ $^ $(INC) $(LIB) $(LINKFLAGS)

.PHONY: clean
clean:
	rm -f *.o timer 

