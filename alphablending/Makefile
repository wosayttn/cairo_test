PROG_NAME = cairo_alphablending
SRCS = cairo_alphablending.c util.c
OBJS = cairo_alphablending.o util.o

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
