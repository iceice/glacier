ROOT_DIR = $(shell pwd)
BIN_DIR = ./cgi-bin

BIN_TARGET = ${BIN_DIR}/add
BIN_SOURCE = ${BIN_DIR}/adder.c

all: server ${BIN_TARGET}

server: tiny.o csapp.o
	gcc -o server tiny.o csapp.o

${BIN_TARGET}: adder.o csapp.o
	gcc -o ${BIN_TARGET} adder.o csapp.o

adder.o: ${BIN_SOURCE}
	gcc -c ${BIN_SOURCE}

tiny.o: tiny.c
	gcc -c tiny.c

csapp.o: csapp.c
	gcc -c csapp.c

clean:
	-rm -rf *.o