CXX			= g++
CXX_FLAGS	= -std=c++11 -ggdb3 -Wall 
CXX_MACRO	=

INC = -I/usr/local/include -I../..
LIB = -L/usr/local/lib -L../../lib -levk -lglog -lpthread

all: addr 

addr: test_inet_addr.cc
	$(CXX) $(CXX_FLAGS) $(CXX_MACRO) -o $@ $^ $(INC) $(LIB)

.PHONY: clean

clean:
	@rm -rf *.o addr 

