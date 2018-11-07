LD_FLAGS:=-ldl	-lpthread
CUSTOM_INC:=-I./inc
OBJECTS:=server.o
DEBUG:=-DDEBUG
CC:=gcc

main:$(OBJECTS)
	$(CC)	-g	$(DEBUG)	$(CUSTOM_INC)	$(OBJECTS)	main.c	-o	main	$(LD_FLAGS)

server.o:
	$(CC)	-g	$(DEBUG)	$(CUSTOM_INC)	-c	src/server.c	$(LD_FLAGS)

clean:
	rm	*.o	main
