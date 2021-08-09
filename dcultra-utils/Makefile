PROG_NAME = dcultra-utils
OBJS = dcultra-utils.o ../dcultra/util.o


CFLAGS += -lm
CFLAGS += -I.

all: ${PROG_NAME}

${PROG_NAME}:${OBJS}
	${CC} -o ${PROG_NAME} ${OBJS} ${LDFLAGS} ${CFLAGS}

${OBJS}:${INCS}

.c.o:
	$(CC) ${CFLAGS} -c -o $*.o $<

clean:
	rm -f *.o ${PROG_NAME}

rebuild: clean all
