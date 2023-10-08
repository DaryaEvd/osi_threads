TARGET1 = a.o
SRCS1 = a.c

# TARGET3 = bebra.o
# SRCS3 = bebra.c

CC=gcc
RM=rm
CFLAGS= -g -Wall
LIBS=-lpthread
INCLUDE_DIR="."

all:
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS1} ${LIBS} -o ${TARGET1}
	# ${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS3} ${LIBS} -o ${TARGET3}

clean:
	${RM} -f *.o ${TARGET1}
	# ${RM} -f *.o ${TARGET3}
