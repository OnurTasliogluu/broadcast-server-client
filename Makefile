LD_FLAGS:=-ldl
CUSTOM_INC:=-I../inc
OBJECTS:=server.o

main:	$(OBJECTS)
	$(CC)	$(LDFLAGS)	-g	$(CUSTOM_INC)	$(OBJECTS)	main.c	$(LD_FLAGS)	-o	main

server.o:
	$(CC)	$(CFLAGS)	-g	-c	server.c	$(CUSTOM_INC)

clean:
	rm	*.o	main