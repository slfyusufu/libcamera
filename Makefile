#
# Makefile
#
# Makefile for fbv


CC	= arm-none-linux-gnueabi-gcc 
CFLAGS := -O2 -Wall -D_GNU_SOURCE -I./include/
SDK_PATH=/home/s100018/mywork/linux/telechips/als-v1.0-r03/build/tcc8971-lcn_v1.0
KERNEL_SOURCE_PATH=$(SDK_PATH)/tmp/work-shared/tcc8971-lcn-2.0/kernel-source
#########for tcc893x#############
#CFLAGS += -I/home/s100018/mywork/linux/linux_platform/bsp/kernel/arch/arm/mach-tcc893x/include
#CFLAGS += -I/home/s100018/mywork/linux/linux_platform/bsp/kernel/usr/include 
#############for tcc897x#########
#CFLAGS += -I/home/s100018/myrepo/linux/linux-als/build/tcc8971-evb/linux/arch/arm/mach-tcc897x/include
#CFLAGS += -I/home/s100018/myrepo/linux/linux-als/build/tcc8971-evb/linux/usr/include
CFLAGS += -I$(KERNEL_SOURCE_PATH)/arch/arm/mach-tcc897x/include
CFLAGS += -I$(KERNEL_SOURCE_PATH)/include/generated
#CFLAGS += -I/home/s100018/myrepo/linux/linux-als/build/tcc8971-evb/linux/usr/include
#################################
#-I/home/s100018/mywork/linux/linux_platform/bsp/kernel/arch/arm/mach-tcc893x/include
LIBS = -L$(PWD)/lib
SOURCES	= camera_test.c camera.c pmap.c
OBJECTS	= ${SOURCES:.c=.o}

#EXECUTABLE	= camera
OUT	= camera
#LIBS	= -lungif -L/usr/X11R6/lib -ljpeg -lpng

all: $(OUT)
	@echo Build DONE.

$(OUT): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(OUT) $(OBJECTS) $(LIBS)

clean:
	rm -f $(OBJECTS) *~ $$$$~* *.bak core config.log $(OUT)


