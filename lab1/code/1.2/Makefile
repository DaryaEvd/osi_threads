TARGET1 = a
SRCS = a.c

TARGET2 = b
SRCS = b.c

TARGET3 = c
SRCS = c.c

TARGET4 = d
SRCS = d.c

# TARGET5 = e
# SRCS = e.c

# TARGET6 = f
# SRCS = f.c

CC=gcc
RM=rm
CFLAGS= -g -Wall
LIBS=-lpthread
INCLUDE_DIR="."

# all: ${TARGET1}
all:
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS} ${LIBS} -o ${TARGET1}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS} ${LIBS} -o ${TARGET2}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS} ${LIBS} -o ${TARGET3}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS} ${LIBS} -o ${TARGET4}
	# ${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS} ${LIBS} -o ${TARGET5}
	# ${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS} ${LIBS} -o ${TARGET6}

clean:
	${RM} -f *.o ${TARGET1}
	${RM} -f *.o ${TARGET2}
	${RM} -f *.o ${TARGET3}
	${RM} -f *.o ${TARGET4}
	# ${RM} -f *.o ${TARGET5}
	# ${RM} -f *.o ${TARGET6}
