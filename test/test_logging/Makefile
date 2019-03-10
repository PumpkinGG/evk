INC = -I/usr/local/include -I ~/Coding
LIB = -L/usr/local/lib

LINKFLAGS = -lglog -lpthread -Wl,-R /usr/local/lib
CXXFLAGS = -std=c++11 -g

all: glog

glog: test_logging.cc
	g++ $(CXXFLAGS) -o $@ $^ $(INC) $(LIB) $(LINKFLAGS)

.PHONY: clean
clean:
	rm -f *.o glog

