INC_PATH = ../lua-5.3.4/src ../protobuf-3.4.1/src
LIB_PATH = ../libs
LIB = lua protobuf pthread

CC = g++
CC_FLAG = -fPIC
LINK_FLAG = -fPIC

PRG = protolua.so
OBJ = proto.o encoder.o decoder.o

all:$(PRG)

$(PRG):$(OBJ)
	$(CC) -shared -o $@ $(OBJ) $(LINK_FLAG) $(LIB_PATH:%=-L%) $(LIB:%=-l%)

.SUFFIXES: .c .o .cpp
.cpp.o:
	$(CC) $(CC_FLAG) $(INC_PATH:%=-I%) -c $*.cpp -o $*.o

.PRONY:clean
clean:
	rm -f $(OBJ) $(PRG)

install:
	cp -f $(PRG) $(LIB_PATH)
