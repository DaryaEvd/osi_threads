TARGET1 = a
SRCS1 = a.c

TARGET2 = b
SRCS2 = b.c

# TARGET3 = c
# SRCS3 = c.c

CC=gcc
RM=rm
CFLAGS= -g -Wall
LIBS=-lpthread
INCLUDE_DIR="."

all:
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS1} ${LIBS} -o ${TARGET1}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS2} ${LIBS} -o ${TARGET2}
	# ${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS3} ${LIBS} -o ${TARGET3}

clean:
	${RM} -f *.o ${TARGET1}
	${RM} -f *.o ${TARGET2}
	# ${RM} -f *.o ${TARGET3}
