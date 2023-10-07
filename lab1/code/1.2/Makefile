TARGET1 = a
SRCS1 = a.c

TARGET2 = b
SRCS2 = b.c

TARGET3 = c
SRCS3 = c.c

TARGET4 = d
SRCS4 = d.c

TARGET5 = e
SRCS5 = e.c

TARGET6 = f
SRCS6 = f.c

CC=gcc
RM=rm
CFLAGS= -g -Wall
LIBS=-lpthread
INCLUDE_DIR="."

# all: ${TARGET1}
all:
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS1} ${LIBS} -o ${TARGET1}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS2} ${LIBS} -o ${TARGET2}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS3} ${LIBS} -o ${TARGET3}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS4} ${LIBS} -o ${TARGET4}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS5} ${LIBS} -o ${TARGET5}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS6} ${LIBS} -o ${TARGET6}

clean:
	${RM} -f *.o ${TARGET1}
	${RM} -f *.o ${TARGET2}
	${RM} -f *.o ${TARGET3}
	${RM} -f *.o ${TARGET4}
	${RM} -f *.o ${TARGET5}
	${RM} -f *.o ${TARGET6}
