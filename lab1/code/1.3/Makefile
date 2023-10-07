TARGET1 = a
SRCS1 = a.c

# TARGET2 = b
# SRCS2 = b.c

CC=gcc
RM=rm
CFLAGS= -g -Wall
LIBS=-lpthread
INCLUDE_DIR="."

all:
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS1} ${LIBS} -o ${TARGET1}
	# ${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS2} ${LIBS} -o ${TARGET2}

clean:
	${RM} -f *.o ${TARGET1}
	# ${RM} -f *.o ${TARGET2}
