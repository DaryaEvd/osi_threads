TARGET_1 = proxy.o
SRCS_1 = main-proxy.c proxy.c network.c parser.c libparser/picohttpparser.c 

CC=gcc
RM=rm
CFLAGS= -g -Wall
LIBS=-lpthread
INCLUDE_DIR="."

all: ${TARGET_1}

${TARGET_1}: proxy.h network.h parser.h libparser/picohttpparser.h  ${SRCS_1}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS_1} ${LIBS} -o ${TARGET_1}

clean:
	${RM} -f *.o ${TARGET_1}
