BASE_PATH = ../..
INC = -I/usr/local/include -I $(BASE_PATH)
LIB = -L/usr/local/lib

CXXFLAGS = -std=c++11 -g
LINKFLAGS = -lglog -lpthread -Wl,-R /usr/local/lib

all: timer 

channel.o: $(BASE_PATH)/evk/channel.cc $(BASE_PATH)/evk/channel.h
	g++ $(CXXFLAGS) -c $< $(INC)

invoke_timer.o: $(BASE_PATH)/evk/invoke_timer.cc $(BASE_PATH)/evk/invoke_timer.h
	g++ $(CXXFLAGS) -c $< $(INC)

timestamp.o: $(BASE_PATH)/evk/timestamp.cc $(BASE_PATH)/evk/timestamp.h
	g++ $(CXXFLAGS) -c $< $(INC)

timer_queue.o: $(BASE_PATH)/evk/timer_queue.cc $(BASE_PATH)/evk/timer_queue.h
	g++ $(CXXFLAGS) -c $< $(INC)

event_loop.o: $(BASE_PATH)/evk/event_loop.cc $(BASE_PATH)/evk/event_loop.h
	g++ $(CXXFLAGS) -c $< $(INC)

poller.o: $(BASE_PATH)/evk/poller.cc $(BASE_PATH)/evk/poller.h
	g++ $(CXXFLAGS) -c $< $(INC)

timer: test_invoke_timer.cc event_loop.o poller.o timer_queue.o \
	channel.o invoke_timer.o timestamp.o
	g++ $(CXXFLAGS) -o $@ $^ $(INC) $(LIB) $(LINKFLAGS)

.PHONY: clean
clean:
	rm -f *.o timer 

