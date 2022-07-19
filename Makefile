ROOTNAME=fsshell
HW=
FOPTION=
RUNOPTIONS=SampleVolume 10000000 512
CC=gcc
CFLAGS= -g -I.
LIBS =pthread
DEPS = 

ADDOBJ= fsLow.o VolumeControlBlock.o FreeSpace.o Directory.o fsInit.o parsePath.o mfs.o b_io.c
ARCH = $(shell uname -m)

ifeq ($(ARCH), aarch64)
	ARCHOBJ=fsLowM1.o
else
	ARCHOBJ=fsLow.o
endif

OBJ = $(ROOTNAME)$(HW)$(FOPTION).o $(ADDOBJ) $(ARCHOBJ)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) 

$(ROOTNAME)$(HW)$(FOPTION): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) -lm -l readline -l $(LIBS)

clean:
	rm $(ROOTNAME)$(HW)$(FOPTION).o $(ADDOBJ) $(ROOTNAME)$(HW)$(FOPTION)

run: $(ROOTNAME)$(HW)$(FOPTION)
	./$(ROOTNAME)$(HW)$(FOPTION) $(RUNOPTIONS)
