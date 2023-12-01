TARGET_1 = list.o
SRCS_1 = list-main.c 
SRCS_2 = list-impl.c

CC=gcc
RM=rm
CFLAGS= -g -Wall
LIBS=-lpthread
INCLUDE_DIR="."

all: ${TARGET_1}

${TARGET_1}: list.h ${SRCS_1} ${SRCS_2}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS_1} ${SRCS_2} ${LIBS} -o ${TARGET_1}

clean:
	${RM} -f *.o ${TARGET_1}
