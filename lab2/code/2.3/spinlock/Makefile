TARGET_1 = list-spinlock.o
SRCS_1_1 = list-main-spinlock.c 
SRCS_1_2 = list-implement-spinlock.c
SRCS_1_3 = stuff.c

# TARGET_2 = list-spinlock.o
# SRCS_2_1 = list-main-spinlock.c 

CC=gcc
RM=rm
CFLAGS= -g -Wall
LIBS=-lpthread
INCLUDE_DIR="."

all: ${TARGET_1}

${TARGET_1}: list.h stuff.h ${SRCS_1_1} ${SRCS_1_2} ${SRCS_1_3} 
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS_1_1} ${SRCS_1_2} ${SRCS_1_3} ${LIBS} -o ${TARGET_1}

# ${TARGET_2}: list.h stuff.h ${SRCS_2_1} ${SRCS_1_2} ${SRCS_1_3} 
# 	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS_2_1} ${SRCS_1_2} ${SRCS_1_3} ${LIBS} -o ${TARGET_2}

clean:
	${RM} -f *.o ${TARGET_1}
