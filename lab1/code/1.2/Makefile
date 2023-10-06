TARGET1 = a
SRCS = a.c

CC=gcc
RM=rm
CFLAGS= -g -Wall
LIBS=-lpthread
INCLUDE_DIR="."

# all: ${TARGET1}
all:
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS} ${LIBS} -o ${TARGET1}

clean:
	${RM} -f *.o ${TARGET1}
