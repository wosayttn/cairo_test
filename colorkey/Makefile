PROG_NAME = cairo_colorkey
OBJS = cairo_colorkey.o ../dcultra/util.o

LDFLAGS += -lfusion
LDFLAGS += -lm
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
