CXX			= g++
CXX_FLAGS	= -std=c++11 -ggdb3 -Wall 
CXX_MACRO	=

INC = -I/usr/local/include -I../..
LIB = -L/usr/local/lib -L../../lib -levk -lglog -lpthread

all: echo

echo: test_thread_pool.cc echo.o
	$(CXX) $(CXX_FLAGS) $(CXX_MACRO) -o $@ $^ $(INC) $(LIB)

echo.o: echo.cc echo.h
	$(CXX) $(CXX_FLAGS) $(CXX_MACRO) -c $<

.PHONY: clean

clean:
	@rm -rf *.o echo

