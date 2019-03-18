#
# product .a lib
#

AR          = ar
AR_FLAGS    = crs

CXX			= g++
CXX_FLAGS	= -std=c++11 -ggdb3 -Wall 
CXX_MACRO	=

BASE_DIR = ./
SRC_DIR  = ./evk
OBJ_DIR  = obj
LIB_DIR  = lib

INC_3RD = -I/usr/local/include
LIB_3RD = -L/usr/local/lib -lglog

INC = -I$(BASE_DIR) $(INC_3RD)
LIB = $(LIB_3RD) -lpthread

all: dir $(LIB_DIR)/libevk.a

# do not print error if dir exists
dir:
	@-mkdir -p $(OBJ_DIR)
	@-mkdir -p $(LIB_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cc
	$(CXX) $(CXX_FLAGS) $(CXX_MACRO) -o $@ -c $(<)

$(LIB_DIR)/libevk.a: $(addprefix $(OBJ_DIR)/, \
	acceptor.o buffer.o channel.o event_loop.o event_loop_thread.o \
	inet_address.o invoke_timer.o poller.o socket.o socket_ops.o \
	tcp_conn.o tcp_server.o timer_queue.o timestamp.o)
	$(AR) $(AR_FLAGS) $(@) $^

.PHONY: clean

clean:
	@rm -rf $(OBJ_DIR)
	@rm -rf $(LIB_DIR)

