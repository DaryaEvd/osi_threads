TARGET1 = example-my-create.o
SRCS1 = myThreadCreate.c main.c

CC=gcc
RM=rm
CFLAGS= -g -Wall
LIBS=-lpthread
INCLUDE_DIR="."

all: ${TARGET1}

${TARGET1}: myThreadCreate.h ${SRCS1}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS1} ${LIBS} -o ${TARGET1}

clean:
	${RM} -f *.o ${TARGET1}