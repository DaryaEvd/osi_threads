TARGET_1 = list-mutex.o
SRCS_1_1 = list-main-mutex.c 
SRCS_1_2 = list-implement-mutex.c
SRCS_1_3 = stuff.c

CC=gcc
RM=rm
CFLAGS= -g -Wall 
LIBS=-lpthread
INCLUDE_DIR="."

all: ${TARGET_1}

${TARGET_1}: list-mutex.h stuff.h ${SRCS_1_1} ${SRCS_1_2} ${SRCS_1_3} 
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS_1_1} ${SRCS_1_2} ${SRCS_1_3} ${LIBS} -o ${TARGET_1}

clean:
	${RM} -f *.o ${TARGET_1}
