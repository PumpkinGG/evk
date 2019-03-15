BASE_PATH = ../..
SRC = $(BASE_PATH)/evk
INC = -I/usr/local/include -I $(BASE_PATH)
LIB = -L/usr/local/lib

CXXFLAGS = -std=c++11 -g
LINKFLAGS = -lglog -lpthread -Wl,-R /usr/local/lib

all: event

timestamp.o: $(SRC)/timestamp.cc $(SRC)/timestamp.h
	g++ $(CXXFLAGS) -c $<

channel.o: $(SRC)/channel.cc $(SRC)/channel.h
	g++ $(CXXFLAGS) -c $<

invoke_timer.o: $(SRC)/invoke_timer.cc $(SRC)/invoke_timer.h
	g++ $(CXXFLAGS) -c $<

timer_queue.o: $(SRC)/timer_queue.cc $(SRC)/timer_queue.h
	g++ $(CXXFLAGS) -c $<

event_loop.o: $(SRC)/event_loop.cc $(SRC)/event_loop.h
	g++ $(CXXFLAGS) -c $<

poller.o: $(SRC)/poller.cc $(SRC)/poller.h
	g++ $(CXXFLAGS) -c $<

socket_ops.o: $(SRC)/socket_ops.cc $(SRC)/socket_ops.h
	g++ $(CXXFLAGS) -c $<

event_loop_thread.o: $(SRC)/event_loop_thread.cc $(SRC)/event_loop_thread.h
	g++ $(CXXFLAGS) -c $<

event: test_event_loop_thread.cc event_loop_thread.o event_loop.o poller.o channel.o \
	timer_queue.o invoke_timer.o  timestamp.o socket_ops.o
	g++ $(CXXFLAGS) -o $@ $^ $(INC) $(LIB) $(LINKFLAGS)

.PHONY: clean
clean:
	rm -f *.o event

