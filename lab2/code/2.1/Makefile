TARGET_1 = queue-example.o
SRCS_1 = queue.c queue-example.c

TARGET_2 = queue-threads.o
SRCS_2 = queue.c queue-threads.c

CC=gcc
RM=rm
CFLAGS= -g -Wall
LIBS=-lpthread
INCLUDE_DIR="."

all: ${TARGET_1} ${TARGET_2}

${TARGET_1}: queue.h ${SRCS_1}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS_1} ${LIBS} -o ${TARGET_1}

${TARGET_2}: queue.h ${SRCS_2}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS_2} ${LIBS} -o ${TARGET_2}

clean:
	${RM} -f *.o ${TARGET_1} ${TARGET_2}
