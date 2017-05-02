LD_FLAGS:=-ldl -lpthread
CUSTOM_INC:=-I./inc
OBJECTS:=server.o

main:	$(OBJECTS)
	$(CC)	$(LDFLAGS)	-g	$(CUSTOM_INC)	$(OBJECTS)	main.c	$(LD_FLAGS)	-o	main

server.o:
	$(CC)	$(CFLAGS)	-g	-c	src/server.c	-I./inc

clean:
	rm	*.o	main