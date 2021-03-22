ROOTFS=/usr/local/oecore-x86_64/sysroots/aarch64-poky-linux/

CC = aarch64-poky-linux-gcc --sysroot=${ROOTFS}
PROG_NAME = cairo_file
SRCS = cairo_file.c
OBJS = cairo_file.o

CFLAGS += -I${ROOTFS}/usr/include/cairo

LDFLAGS += -lcairo

all: ${PROG_NAME}

${PROG_NAME}:${OBJS}
	${CC} -o ${PROG_NAME} ${OBJS} ${LDFLAGS} ${CFLAGS}

${OBJS}:${INCS}

.c.o:
	$(CC) ${CFLAGS} -c -o $*.o $<

clean:
	rm -f *.o ${PROG_NAME}

rebuild: clean all	
