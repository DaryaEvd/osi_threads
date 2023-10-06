TARGET1 = a
SRCS = a.c

TARGET2 = b
SRCS = b.c

CC=gcc
RM=rm
CFLAGS= -g -Wall
LIBS=-lpthread
INCLUDE_DIR="."

# all: ${TARGET1}
all:
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS} ${LIBS} -o ${TARGET1}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS} ${LIBS} -o ${TARGET2}

clean:
	${RM} -f *.o ${TARGET1}
	${RM} -f *.o ${TARGET2}
