export SYSROOT=${SDKTARGETSYSROOT}
export CC = aarch64-poky-linux-gcc --sysroot=${SYSROOT}
export CFLAGS = -I${SYSROOT}/usr/include/cairo
export LDFLAGS += -lcairo
