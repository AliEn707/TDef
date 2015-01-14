
NORMAL_FLAGS = -lm -g -Wall -lpthread -fsigned-char
OPTIMIZED_FLAGS = -march=native -lm -Wall -lpthread -O3 -ffast-math
C_FLAGS = $(NORMAL_FLAGS)

ifeq ($(build),release)
	C_FLAGS = $(OPTIMIZED_FLAGS)
endif

server:	simple.c ../src/*.c ../src/*.h
	gcc simple.c ../src/*.c $(C_FLAGS) -c
	mv *.o objects/
	gcc objects/*.o $(C_FLAGS) -o server
	
manager: manager.c debug.c manager.h
	gcc manager.c debug.c -g -Wall -lpthread -fsigned-char -o manager

daemon: manager.c daemon.c manager.h
	gcc daemon.c manager.c -g -Wall -lpthread -fsigned-char -o daemon
	
all: server manager daemon

clean:
	rm -f server manager daemon objects/*.o

