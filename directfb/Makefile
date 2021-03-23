PROG_NAME = cairo_directfb
SRCS = cairo_directfb.c
OBJS = cairo_directfb.o

CFLAGS += -I${SYSROOT}/usr/include/directfb

LDFLAGS += -ldirectfb
LDFLAGS += -ldirect
LDFLAGS += -lfusion
LDFLAGS += -lz

all: ${PROG_NAME}

${PROG_NAME}:${OBJS}
	${CC} -o ${PROG_NAME} ${OBJS} ${LDFLAGS} ${CFLAGS}

${OBJS}:${INCS}

.c.o:
	$(CC) ${CFLAGS} -c -o $*.o $<

clean:
	rm -f *.o ${PROG_NAME}

rebuild: clean all
